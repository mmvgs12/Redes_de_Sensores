#include <Wire.h>
#include <Arduino_LSM9DS1.h>

#define I2C_SLAVE_ADDR 0x55

const int BUF_SIZE = 5;

struct ImuSample {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
};

ImuSample buf[BUF_SIZE];
bool dataReady = false;

void onRequest() {
  if (dataReady) {
    Wire.write((uint8_t*)buf, sizeof(buf));
    dataReady = false;
  } else {
    uint8_t empty[sizeof(buf)] = {0};
    Wire.write(empty, sizeof(empty));
  }
}

void capturar() {
  for (int i = 0; i < BUF_SIZE; i++) {
    float x, y, z;
    IMU.readAcceleration(x, y, z);  buf[i].ax = x; buf[i].ay = y; buf[i].az = z;
    IMU.readGyroscope(x, y, z);     buf[i].gx = x; buf[i].gy = y; buf[i].gz = z;
    IMU.readMagneticField(x, y, z); buf[i].mx = x; buf[i].my = y; buf[i].mz = z;
    delay(200);
  }
  dataReady = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(onRequest);

  if (!IMU.begin()) {
    Serial.println("ERROR: IMU no encontrada");
    while (1);
  }
}

void loop() {
  // Captura continua — siempre hay datos frescos disponibles para el ESP32
  capturar();
}