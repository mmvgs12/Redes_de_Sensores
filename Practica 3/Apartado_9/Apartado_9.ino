
#include <WiFi.h>
#include "time.h"
#include <ArduinoJson.h>

// ─── WiFi ────────────────────────────────────────────
const char* ssid = "DIGIFIBRA-bGf3";
const char* password = "h7FcER-135/zdLS;8BgAp2:V";

// ─── NTP ─────────────────────────────────────────────
const char* TZ_INFO   = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* ntpServer = "europe.pool.ntp.org";

// ─── FTP ─────────────────────────────────────────────
const char* ftp_server = "192.168.1.23";  // tu IP local
const int   ftp_port   = 21;
const char* ftp_user   = "Test_1";
const char* ftp_pass   = "Test_1";

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO = 10000; // 10 segundos

// ─── NTP: obtener hora formateada ────────────────────
String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "1970-01-01T00:00:00";
  }
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buf);
}

// ─── Nombre de archivo con hora real ─────────────────
String getNombreArchivo() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "grupo01_000000.json";
  }
  char buf[32];
  sprintf(buf, "grupo01_%02d%02d%02d.json",
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec);
  return String(buf);
}

// ─── Generar JSON SenML ───────────────────────────────
String generarSenML() {
  float temperatura = 20.0 + (random(0, 1000) / 100.0);

  time_t ahora;
  time(&ahora);

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  JsonObject base = arr.add<JsonObject>();
  base["bn"]  = "urn:dev:esp32c6:sensor:";
  base["bt"]  = (long)ahora;          // timestamp Unix real
  base["ver"] = 10;

  JsonObject medida = arr.add<JsonObject>();
  medida["n"] = "temperatura";
  medida["u"] = "Cel";
  medida["v"] = temperatura;

  String output;
  serializeJson(doc, output);
  return output;
}

// ─── FTP helpers ─────────────────────────────────────
String ftpResponse(WiFiClient& client) {
  String resp = "";
  unsigned long t = millis();
  while (millis() - t < 3000) {
    while (client.available()) {
      resp += (char)client.read();
    }
    if (resp.length() > 0) break;
    delay(10);
  }
  Serial.println("FTP< " + resp);
  return resp;
}

void ftpCommand(WiFiClient& client, String cmd) {
  Serial.println("FTP> " + cmd);
  client.println(cmd);
  ftpResponse(client);
}

// ─── Subir archivo por FTP ────────────────────────────
void enviarFTP() {
  String jsonStr  = generarSenML();
  String filename = getNombreArchivo();

  Serial.println("JSON: " + jsonStr);
  Serial.println("Subiendo: " + filename);

  WiFiClient control;
  if (!control.connect(ftp_server, ftp_port)) {
    Serial.println("Error conectando al servidor FTP");
    return;
  }
  ftpResponse(control);

  ftpCommand(control, "USER " + String(ftp_user));
  ftpCommand(control, "PASS " + String(ftp_pass));
  ftpCommand(control, "TYPE I");

  control.println("PASV");
  Serial.println("FTP> PASV");
  String pasv = ftpResponse(control);

  int inicio = pasv.indexOf('(');
  int fin    = pasv.indexOf(')');
  String nums = pasv.substring(inicio + 1, fin);

  int n[6];
  int pos = 0;
  for (int i = 0; i < 6; i++) {
    int coma = nums.indexOf(',', pos);
    if (coma == -1) coma = nums.length();
    n[i] = nums.substring(pos, coma).toInt();
    pos = coma + 1;
  }
  int dataPort = n[4] * 256 + n[5];
  String dataIP = String(n[0]) + "." + String(n[1]) + "." +
                  String(n[2]) + "." + String(n[3]);

  WiFiClient datos;
  if (!datos.connect(dataIP.c_str(), dataPort)) {
    Serial.println("Error conectando canal de datos");
    control.stop();
    return;
  }

  ftpCommand(control, "STOR " + filename);
  delay(200);

  datos.print(jsonStr);
  datos.flush();
  datos.stop();

  delay(500);
  ftpResponse(control);
  ftpCommand(control, "QUIT");
  control.stop();

  Serial.println("Subido correctamente: " + filename);
}

// ─── Setup ───────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConectado!");

  // NTP
  configTzTime(TZ_INFO, ntpServer);
  Serial.print("Sincronizando NTP");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHora: " + getTimestamp());

  ultimoEnvio = millis();
}

// ─── Loop ────────────────────────────────────────────
void loop() {
  if (millis() - ultimoEnvio >= INTERVALO) {
    ultimoEnvio = millis();
    enviarFTP();
  }
}