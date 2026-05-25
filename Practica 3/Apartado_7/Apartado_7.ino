
#include "WiFi.h"
#include "Webpage.h"

// ⚠️ Cambia estos datos
const char* ssid = "DIGIFIBRA-bGf3";
const char* password = "h7FcER-135/zdLS;8BgAp2:V";

WiFiServer server(80);
unsigned long tiempoBase = 0;

String getHora() {
  unsigned long segundos = (millis() - tiempoBase) / 1000;
  unsigned long horas    = segundos / 3600;
  unsigned long minutos  = (segundos % 3600) / 60;
  unsigned long segs     = segundos % 60;
  char buf[9];
  sprintf(buf, "%02lu:%02lu:%02lu", horas, minutos, segs);
  return String(buf);
}

String getHTML() {
  String html = String(index_html);
  html.replace("HORA_ACTUAL", getHora());
  return html;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nIP: " + WiFi.localIP().toString());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  // Espera a que el cliente envíe datos
  while (!client.available()) delay(1);

  // Lee la primera línea del request
  String request = client.readStringUntil('\r');
  client.flush();

  // Decide qué responder
  String response = "";
  String contentType = "";

  if (request.indexOf("GET /reset") >= 0) {
    tiempoBase = millis();
    response    = "ok";
    contentType = "text/plain";

  } else if (request.indexOf("GET /hora") >= 0) {
    response    = getHora();
    contentType = "text/plain";

  } else {
    // Página principal
    response    = getHTML();
    contentType = "text/html";
  }

  // Envía la respuesta HTTP
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: " + contentType);
  client.println("Connection: close");
  client.println();
  client.println(response);

  delay(1);
  client.stop();
}