#include <Wire.h>
#include <Adafruit_NeoPixel.h>

//Parametros del LED del ESP32-C6
#define NANO_ADDR  0x55
#define NEO_PIN    8
#define NUM_LEDS   1

Adafruit_NeoPixel strip(NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);

const int BUF_SIZE = 5;

struct ImuSample {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
};

void setup() {
  Serial.begin(115200);
  //Inicializa y apaga el LED
  strip.begin();
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show();
  //Inicializa el i2c
  Wire.begin(6, 7);

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

      // Enciende LED
      strip.setPixelColor(0, strip.Color(0, 50, 0));
      strip.show();

      // Muestra todas las muestras
      Serial.println("--- Muestras IMU recibidas ---");
      for (int i = 0; i < BUF_SIZE; i++) {
        char line[128];
        sprintf(line,
          "[%d] Acc(%.3f,%.3f,%.3f) Gyr(%.3f,%.3f,%.3f) Mag(%.3f,%.3f,%.3f)",
          i,
          buf[i].ax, buf[i].ay, buf[i].az,
          buf[i].gx, buf[i].gy, buf[i].gz,
          buf[i].mx, buf[i].my, buf[i].mz
        );
        Serial.println(line);
      }
      Serial.println("-----------------------------");
      //Pasado 1 segundo, apaga el LED
      delay(1000);
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
    }
  }

  delay(200);
}