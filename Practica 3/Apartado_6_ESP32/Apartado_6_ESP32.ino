#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "time.h"

//Parametros del LED del ESP32-C6
#define NANO_ADDR  0x55
#define NEO_PIN    8
#define NUM_LEDS   1

Adafruit_NeoPixel strip(NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);

const int BUF_SIZE = 5;

//Datos del wifi
const char* ssid = "POCO X4 GT";
const char* password = "hcun4ezw8y3wd4b";
const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* ntpServer = "europe.pool.ntp.org";
IPAddress server_ip(10,189,115,97); 
const uint16_t port = 5000;
WiFiClient wifi_client;

//estructura de datos para el IMU
struct ImuSample {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
};

String printDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  
  }
  char formattedTime[80];  // Buffer to store the formatted string
  strftime(formattedTime, sizeof(formattedTime), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.println(formattedTime);
  return formattedTime;
}

void setup() {
  Serial.begin(115200);
  //Inicializa y apaga el LED
  strip.begin();
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show();
  //Inicializa el i2c
  Wire.begin(6, 7);
  //Inicializa el wifi
   WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //Conexion a la red wifi
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("\nConnected to WiFi!");
  //Configuracion del servidor ntp
  configTzTime(TZ_INFO, ntpServer);
  Serial.println("NTP time configured.");
  Serial.print("Connecting to client...");

  //Conectar TCP
  Serial.print("Connecting to TCP server...");
  while (!wifi_client.connect(server_ip, port)) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println("\nTCP connected!");

  while(wifi_client.connected() == 0) {
    if (wifi_client.connect(server_ip, port) == 1) {
      Serial.println("¡Conectado!");
    }
    delay(1000);
  }
  

}

void loop() {
  int totalBytes = sizeof(ImuSample) * BUF_SIZE;
  //Solicita al nano via i2c informacion
  Wire.requestFrom(NANO_ADDR, totalBytes);
  //Si hay datos disponibles, muestra los datos por serial y enciende el led
  if (Wire.available() >= totalBytes) {
    ImuSample buf[BUF_SIZE];
    Wire.readBytes((uint8_t*)buf, totalBytes);

    // Comprueba que no es buffer vacío
    if (buf[0].ax != 0 || buf[0].ay != 0 || buf[0].az != 0) {

      int i = 0;
      if (wifi_client.available() > 0) {
      // Envía cada muestra por WiFi
      String timestamp = printDateTime();
      for (int i = 0; i < BUF_SIZE; i++) {
        String payload = timestamp
          + ";" + String(i)
          + ";" + String(buf[i].ax, 4)
          + ";" + String(buf[i].ay, 4)
          + ";" + String(buf[i].az, 4);

        wifi_client.println(payload);
        Serial.println("Sent: " + payload);
      }

      delay(1000);
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
    }
  }
  }
  delay(200);
}