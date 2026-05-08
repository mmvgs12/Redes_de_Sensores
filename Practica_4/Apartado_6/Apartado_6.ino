#include <WiFi.h>
#include "time.h"

const char* ssid = "POCO X4 GT";
const char* password = "hcun4ezw8y3wd4b";
const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* ntpServer = "europe.pool.ntp.org";
IPAddress server_ip(10,189,115,97); 
const uint16_t port = 500;
WiFiClient wifi_client;

bool Sending = false;

// Function that prints formatted date and time
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
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
    }
  Serial.println("\nConnected to WiFi!");
  configTzTime(TZ_INFO, ntpServer);
  Serial.println("NTP time configured.");
  Serial.print("Connecting to client...");

  while(wifi_client.connected() == 0) {
    if (wifi_client.connect(server_ip, port) == 1) {
      Serial.println("¡Conectado!");
    }
    delay(1000);
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  // Print formatted date and time
  //printDateTime();

  if (wifi_client.available() > 0) {
    Serial.println("Mensaje recibido del host");
    String mensaje = wifi_client.readStringUntil('\n');
    mensaje.trim();
    Serial.println("Recibido: " + mensaje);

    Serial.println(mensaje);
    if (mensaje == "START"){
      Sending = true;
    } else if (mensaje == "STOP"){
      Sending = false;
    }
  }
  if (Sending) {
    String timestamp = printDateTime();
    wifi_client.println(timestamp);
    Serial.println("Enviado: " + timestamp);
    delay(1000);
  }
}
