#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

const char* DEVICE_NAME = "NanoIoT-Accel_S_M";

BLEService imuService("1101");
// Característica solo lectura + notificaciones: acelerómetro
BLEStringCharacteristic accelChar("2101", BLERead | BLENotify, 48);
// Característica solo escritura: activar/desactivar
BLEStringCharacteristic enableChar("2102", BLEWrite, 4);

bool accelEnabled = true;

void setup() {
  Serial.begin(115200);
  IMU.begin();
  BLE.begin();
  BLE.setLocalName(DEVICE_NAME);
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(accelChar);
  imuService.addCharacteristic(enableChar);
  BLE.addService(imuService);
  BLE.advertise();

}

void loop() {
  BLEDevice central = BLE.central();

  while (central.connected()) {
    // Procesar comando enable/disable
    if (enableChar.written()) {
      String read = enableChar.value();
      read.toUpperCase();
      Serial.print(read);

      if(read == "T")
      {
        accelEnabled = true;
      } else if (read == "F")
      {
        accelEnabled = false;
      }
    }

    if (accelEnabled && IMU.accelerationAvailable()) {
      float ax, ay, az;
      IMU.readAcceleration(ax, ay, az);
      char buf[48];
      
      Serial.println("Los valores del IMU son: ");
      Serial. println("eje x: " + String(ax));
      Serial. println("eje y: " + String(ay));
      Serial. println("eje z: " + String(az));
      Serial.println("");
      
      snprintf(buf, sizeof(buf), "%.4f,%.4f,%.4f", ax, ay, az);
      accelChar.writeValue(String(buf));
    }
    delay(100); // actualizar cada 100ms
  }
  Serial.println("Desconectado.");
}