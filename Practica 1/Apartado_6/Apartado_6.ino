
#include <Arduino_LSM9DS1.h>

const int BUF_SIZE = 10;

struct ImuSample {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
};


ImuSample buf[BUF_SIZE];

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("ERROR: IMU no encontrada");
    while (1); // Si el IMU no esta, detiene el programa
  }

}

void loop() {
  // Toma 10 muestras cada 100ms
  for (int i = 0; i < BUF_SIZE; i++) {
    IMU.readAcceleration(buf[i].ax, buf[i].ay, buf[i].az);
    IMU.readGyroscope(buf[i].gx,   buf[i].gy, buf[i].gz);
    IMU.readMagneticField(buf[i].mx, buf[i].my, buf[i].mz);
    delay(100);
  }

  // Envía todas las muestras por UART
  Serial.println("--- Muestras IMU ---");
  for (int i = 0; i < BUF_SIZE; i++) {
    char line[128];
    sprintf(line,
      "[%2d] Acc(%.3f,%.3f,%.3f) Gyr(%.3f,%.3f,%.3f) Mag(%.3f,%.3f,%.3f)",
      i,
      buf[i].ax, buf[i].ay, buf[i].az,
      buf[i].gx, buf[i].gy, buf[i].gz,
      buf[i].mx, buf[i].my, buf[i].mz
    );
    Serial.println(line);
  }
  Serial.println("--------------------");
}