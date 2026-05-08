#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
const char* BT_NAME = "ESP32-Chat-IoT";

void setup() {
  Serial.begin(115200);
  SerialBT.begin(BT_NAME);
  Serial.printf("Bluetooth iniciado como: %s\n", BT_NAME);
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    SerialBT.println(msg);
    Serial.printf("[ESP32→BT] %s\n", msg.c_str());
  }

  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    msg.trim();
    Serial.printf("[BT→ESP32] %s\n", msg.c_str());
    SerialBT.printf("Echo: %s\n", msg.c_str());
  }
}