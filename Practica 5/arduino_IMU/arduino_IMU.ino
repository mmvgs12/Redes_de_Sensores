#include <Arduino_LSM9DS1.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);  // Espera a que se abra el monitor serial

  if (!IMU.begin()) {
    Serial.println("Error al iniciar el IMU");
    while (1);
  }

  Serial.println("ax(g),ay(g),az(g)");  // Cabecera CSV
}

void loop() {
  float ax, ay, az;

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);

    Serial.print(ax);
    Serial.print(",");
    Serial.print(ay);
    Serial.print(",");
    Serial.println(az);
  }

  delay(100);  // 10 lecturas por segundo
}