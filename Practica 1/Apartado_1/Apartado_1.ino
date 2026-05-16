
const int ADC_PIN = A0;
int raw = 0;
float voltage = 0;
char buf[64];

void setup() {
  Serial.begin(115200);
  while (!Serial);
  analogReadResolution(12); // Nano 33 BLE soporta 12 bits (0-4095)
  Serial.println("=== Lectura ADC cada 1 segundo ===");
}

void loop() {

  raw = analogRead(ADC_PIN);
  voltage = raw * 3.3f / 4095.0f;
  sprintf(buf, "ADC raw: %4d  |  Tensión: %.3f V", raw, voltage);
  Serial.println(buf);
  delay(1000);
}
