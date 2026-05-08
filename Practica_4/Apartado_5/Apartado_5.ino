#include "BluetoothSerial.h"

const char* ADVERTISING_NAME = "ESP32-Chat-IoT"; //Advertising del dispositivo
BluetoothSerial SerialBT; //crea el objeto BluetoothSerial

void setup() {
  Serial.begin(115200);
  if (!SerialBT.begin(ADVERTISING_NAME)) {
    Serial.println("No se ha podido iniciar bluetooth");
    while(1); // Si falla la conexion del bluetooth, "para" el programa
  }
  Serial.printf("Bluetooth iniciado como: %s\n", ADVERTISING_NAME);
}

void loop() {

  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n'); //lee el mensaje recibido por serial hasta qie hay un salto de linea
    msg.trim(); //elimina los saltos del linea
    SerialBT.println(msg); //Manda el mensaje por bluetooth al smartphone
    Serial.printf("[ESP32] %s\n", msg.c_str()); //imprime por pantalla el mensaje enviado por el ESP32
  }

  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n'); //Lee el mensaje que viene del smartphone
    msg.trim();  //elimina los saltos de linea
    Serial.printf("[SMART] %s\n", msg.c_str()); //Imprime por pantalla el mensaje enviado por el Smartphone
    SerialBT.printf("Echo: %s\n", msg.c_str()); //Manda al smarthpone un eco del mensaje recibido
  }
}