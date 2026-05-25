#include <WiFi.h>
#include "time.h"
#include "AdafruitIO_WiFi.h"

// ─── WiFi ────────────────────────────────────────────
const char* ssid = "DIGIFIBRA-bGf3";
const char* password = "h7FcER-135/zdLS;8BgAp2:V";

// ─── Adafruit IO ──────────────────────────────────────
#define AIO_USERNAME "mms12"
#define AIO_KEY      ""
// ─── NTP ─────────────────────────────────────────────
const char* TZ_INFO   = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* ntpServer = "europe.pool.ntp.org";

// ─── Adafruit IO feed ─────────────────────────────────
AdafruitIO_WiFi io(AIO_USERNAME, AIO_KEY, ssid, password);
AdafruitIO_Feed *feedTemperatura = io.feed("temperatura");

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO = 10000;

// ─── Callback: se ejecuta al recibir dato del feed ───
void mensajeRecibido(AdafruitIO_Data *data) {
  Serial.println("─────────────────────────────────");
  Serial.println(">>> Dato recibido desde Adafruit IO");
  Serial.print("    Valor: ");
  Serial.println(data->value());
  Serial.println("─────────────────────────────────");
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConectado!");

  configTzTime(TZ_INFO, ntpServer);
  Serial.print("Sincronizando NTP");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHora sincronizada.");

  Serial.print("Conectando a Adafruit IO");
  io.connect();

  // Suscribirse al feed y asignar callback
  feedTemperatura->onMessage(mensajeRecibido);

  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n" + String(io.statusText()));

  // Pide el último valor almacenado al conectar
  feedTemperatura->get();

  ultimoEnvio = millis();
}

void loop() {
  io.run(); // mantiene MQTT activo y dispara callbacks

  if (millis() - ultimoEnvio >= INTERVALO) {
    ultimoEnvio = millis();

    float temperatura = 20.0 + (random(0, 1000) / 100.0);
    time_t ahora;
    time(&ahora);

    Serial.print("Publicando: ");
    Serial.print(temperatura);
    Serial.print(" °C  |  timestamp: ");
    Serial.println((long)ahora);

    feedTemperatura->save(temperatura);
  }
}