// ============================================================
// PRÁCTICA 1 - Apartado 5 - MASTER
// ESP32-C6 — recibe comandos UART y controla LED integrado
// El Nano es el slave que reenvía los comandos
// ============================================================

#include <Wire.h>
#include <Adafruit_NeoPixel.h>

//Configuracion para enceder el led de la placa
#define NEO_PIN  8
#define NUM_LEDS 1
Adafruit_NeoPixel strip(NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);

#define NANO_ADDR 0x08

void setup() {
  Serial.begin(115200);
  //apaga el led al iniciar
  strip.begin();
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show();
  //Establece el i2c en los pines 6 y 7 del ESP32
  Wire.begin(6, 7);  // ESP32-C6 como master

  Serial.println("=== Master I2C listo ===");
}

void loop() {
  // Solicita 1 byte al Nano
  Wire.requestFrom(NANO_ADDR, 1);
  //Cuando el nano le devuelve el valor, comprueba que sea un 1 para encender el led
  if (Wire.available()) {
    char c = Wire.read();
    if (c == '1') {
      strip.setPixelColor(0, strip.Color(0, 0, 50));  //azul
      strip.show();
      Serial.println("LED ON");
    } else if (c == '0') {
      strip.setPixelColor(0, strip.Color(0, 0, 0));   // apagado
      strip.show();
      Serial.println("LED OFF");
    }
  }
  delay(100);
}