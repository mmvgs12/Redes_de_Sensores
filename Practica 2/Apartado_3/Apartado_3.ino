#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "esp_pm.h"
#include "esp_sleep.h"

// NeoPixel: LED RGB integrado en el ESP32-C6 (GPIO8)
#define NEO_PIN  8
#define NUM_LEDS 1
Adafruit_NeoPixel strip(NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// Dirección I2C del Nano 33 BLE configurado como esclavo
#define NANO_ADDR 0x55

// Mutex para proteger el acceso concurrente al buffer entre taskSample y taskSend
SemaphoreHandle_t dataMutex;

// Buffer circular de 10 muestras (100ms × 10 = 1s de datos)
const int BUF_SIZE = 10;
struct AccelSample { float ax, ay, az; };
AccelSample accelBuf[BUF_SIZE];
int bufIndex = 0; // índice de escritura, se resetea tras cada envío

// Idle Hook: se ejecuta automáticamente por FreeRTOS cuando no hay ninguna tarea
// activa (todas están bloqueadas en vTaskDelay). Pone el CPU en light sleep,
// reduciendo el consumo hasta que el próximo vTaskDelay expire y haya trabajo.
// Light sleep mantiene la RAM y los periféricos activos, por lo que la ejecución
// se reanuda de forma transparente sin reiniciar el programa.
void vApplicationIdleHook(void) {
  esp_light_sleep_start();
}

// Solicita 12 bytes al Nano por I2C (3 floats × 4 bytes = ax, ay, az)
// Devuelve false si el Nano no responde o los datos son insuficientes
bool nanoRead(float &ax, float &ay, float &az) {
  Wire.requestFrom(NANO_ADDR, 12);
  if (Wire.available() < 12) return false;
  Wire.readBytes((uint8_t*)&ax, 4);
  Wire.readBytes((uint8_t*)&ay, 4);
  Wire.readBytes((uint8_t*)&az, 4);
  return true;
}

// Tarea 1 (prioridad 2 — alta): muestrea el IMU cada 100ms via I2C
// Durante el vTaskDelay el CPU entra en light sleep via idleHook
void taskSample(void *pvParameters) {
  for (;;) {
    float ax, ay, az;
    if (nanoRead(ax, ay, az)) {
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        accelBuf[bufIndex % BUF_SIZE] = {ax, ay, az};
        bufIndex++;
        xSemaphoreGive(dataMutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // CPU duerme aquí hasta el siguiente muestreo
  }
}

// Tarea 2 (prioridad 1 — baja): cada segundo copia el buffer, lo imprime por Serial
// y parpadea el LED verde durante 200ms como indicador visual de envío
// Durante el vTaskDelay de 1000ms el CPU entra en light sleep via idleHook
void taskSend(void *pvParameters) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000)); // CPU duerme aquí ~900ms de cada segundo

    // Indicador visual: LED verde durante 200ms
    strip.setPixelColor(0, strip.Color(0, 50, 0));
    strip.show();

    // Copiar buffer bajo mutex para no interferir con taskSample
    AccelSample snapshot[BUF_SIZE];
    int count = 0;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      count = min(bufIndex, BUF_SIZE);
      memcpy(snapshot, accelBuf, count * sizeof(AccelSample));
      bufIndex = 0; // resetear índice tras copiar
      xSemaphoreGive(dataMutex);
    }

    // Enviar muestras acumuladas por Serial al PC
    Serial.printf("--- t=%lus | %d muestras ---\n", xTaskGetTickCount() / 1000UL, count);
    for (int i = 0; i < count; i++) {
      Serial.printf("  [%2d] ax=%.003f ay=%.003f az=%.003f\n", i, snapshot[i].ax, snapshot[i].ay, snapshot[i].az);
    }

    // Apagar LED tras 200ms
    vTaskDelay(pdMS_TO_TICKS(200));
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();
  }
}

void setup() {
  Serial.begin(115200);

  // Iniciar NeoPixel y apagar por defecto
  strip.begin();
  strip.show();

  // Iniciar I2C como maestro en los pines correctos del ESP32-C6
  // GPIO6 = SDA, GPIO7 = SCL
  Wire.begin(6, 7);

  // Verificar que el Nano está conectado y responde en la dirección 0x55
  Wire.beginTransmission(NANO_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("ERROR: Nano no encontrado en I2C.");
    while (1) vTaskDelay(pdMS_TO_TICKS(100));
  }
  Serial.println("Nano esclavo I2C OK");

  // Configurar gestión de energía:
  // - max_freq_mhz: frecuencia máxima cuando hay tareas activas
  // - min_freq_mhz: frecuencia mínima durante idle (light sleep)
  // - light_sleep_enable: permite al gestor entrar en light sleep automáticamente
  esp_pm_config_t pm_config = {
    .max_freq_mhz = 160,
    .min_freq_mhz = 10,
    .light_sleep_enable = true
  };
  esp_pm_configure(&pm_config);

  // Crear mutex antes de lanzar las tareas para evitar accesos sin protección
  dataMutex = xSemaphoreCreateMutex();

  // taskSample con prioridad 2 (mayor) para garantizar muestreo preciso a 100ms
  // taskSend con prioridad 1 (menor) ya que el envío puede tolerar pequeños retrasos
  xTaskCreate(taskSample, "Sample", 3072, NULL, 2, NULL);
  xTaskCreate(taskSend,   "Send",   3072, NULL, 1, NULL);
}

// loop() vacío: FreeRTOS gestiona toda la ejecución mediante las tareas creadas
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}