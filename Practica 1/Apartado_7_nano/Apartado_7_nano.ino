#include <Wire.h>
#include <Arduino_LSM9DS1.h>

//define la direccion del i2c
#define I2C_SLAVE_ADDR 0x55

const int BUF_SIZE = 5;

struct ImuSample {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
};

ImuSample buf[BUF_SIZE];
bool dataReady = false;

//Funcion cada vez que el ESP32 pide datos
void onRequest() {
  //Si los datos estan disponibles, manda los datos almacenados durante 1 segundo
  if (dataReady) {
    Wire.write((uint8_t*)buf, sizeof(buf));
    dataReady = false;
  } else {
    // Sin datos: envía buffer vacío
    uint8_t empty[sizeof(buf)] = {0};
    Wire.write(empty, sizeof(empty));
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  //Inicializa el I2C en la posicion deseada
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(onRequest);

  if (!IMU.begin()) {
    Serial.println("ERROR: IMU no encontrada");
    while (1); //Si el IMU no esta disponible, detiene el programa
  }

}

void loop() {
  //Detecta si se ha recibido un mensaje por serial y si es asi lo interpreta
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    //Si el mensaje recibido es CAPTURA, empieza a capturar las 5 medidas (200 ms) del IMU y lo almacena en un buffer
    if (cmd == "CAPTURA") {
      Serial.println("Capturando...");
      for (int i = 0; i < BUF_SIZE; i++) {
        float x, y, z;
        IMU.readAcceleration(x, y, z);  buf[i].ax = x; buf[i].ay = y; buf[i].az = z;
        IMU.readGyroscope(x, y, z);     buf[i].gx = x; buf[i].gy = y; buf[i].gz = z;
        IMU.readMagneticField(x, y, z); buf[i].mx = x; buf[i].my = y; buf[i].mz = z;
        delay(200);
      }
      //Indica a la interrupcion del i2c que tiene los datos disponibles
      dataReady = true;
      Serial.println("Captura completa");
    }
  }
}