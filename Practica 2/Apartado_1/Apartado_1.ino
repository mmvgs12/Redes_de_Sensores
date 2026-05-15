
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const int LED_PIN = 8; // LED integrado en la mayoría de placas ESP32
const int NUM_LEDS = 1;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800); // Necesario para funcionar en el ESP32-C6

//Tarea 1: Parpadeo de LED cada 200 ms 
void taskBlink(void *pvParameters) {
  for (;;) {
    strip.setPixelColor(0, strip.Color(0, 50, 0)); // green, low brightness
    strip.show();
    vTaskDelay(pdMS_TO_TICKS(200));
    strip.setPixelColor(0, strip.Color(0, 0, 0));  // off
    strip.show();
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

//Tarea 2: "Hola mundo" por UART cada 1 s
void taskUART(void *pvParameters) {
  uint32_t count = 0;
  for (;;) {
    Serial.printf("[%.3fs] Hola mundo! (iteración %u)\n",
                  xTaskGetTickCount() / 1000.0f, ++count);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // xTaskCreate(función, nombre, stack, parámetro, prioridad, handle)
  xTaskCreate(taskBlink, "Blink", 2048, NULL, 1, NULL); 
  xTaskCreate(taskUART,  "UART",  2048, NULL, 2, NULL);

}

void loop() {
  // Vacío: todo lo gestiona FreeRTOS
  vTaskDelay(pdMS_TO_TICKS(1000));
}
