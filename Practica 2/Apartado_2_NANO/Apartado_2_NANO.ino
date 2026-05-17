#include <Wire.h>
#include <Arduino_LSM9DS1.h>

// Dirección I2C del Nano 33 BLE configurado como esclavo
#define I2C_SLAVE_ADDR 0x55

volatile float ax = 0, ay = 0, az = 0;
//Funcion para mandar por i2c los datos del acelerometro cada vez que los pide el ESP32
void onRequest() {
  Wire.write((uint8_t*)&ax, 4);
  Wire.write((uint8_t*)&ay, 4);
  Wire.write((uint8_t*)&az, 4);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Wire (esclavo) ANTES que IMU/Wire1
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(onRequest);

  if (!IMU.begin()) {
    Serial.println("ERROR: IMU no encontrada");
    while (1);
  }

  Serial.println("Nano listo");
}

void loop() {
  //Lee el acelerometro y guarda los datos 
  float x, y, z;
  IMU.readAcceleration(x, y, z);
  ax = x;
  ay = y;
  az = z;
  delay(100);
}