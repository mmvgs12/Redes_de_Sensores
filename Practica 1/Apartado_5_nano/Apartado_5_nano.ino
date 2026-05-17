
#include <Wire.h>
//Necesario que el Arduino sea el esclavo y el ESP32 el maestro, porque si no es necesario incluir las resistencias de pull-up
#define I2C_SLAVE_ADDR 0x08 //Direccion del Arduino

volatile char ledCmd = '0';  // último comando recibido por UART

//Request del ESP32
void onRequest() {
  Wire.write(ledCmd);  // ESP32 pregunta → Nano responde con el comando
}

//Interprete del comando que se manda por serial
void parseCommand(String cmd) {
  cmd.trim();
  if (cmd == "LED_ON") {
    ledCmd = '1';
    Serial.println("INFO: LED ON");
  } else if (cmd == "LED_OFF") {
    ledCmd = '0';
    Serial.println("INFO: LED OFF");
  } else {
    Serial.println("ERROR: comando desconocido. Usa LED_ON | LED_OFF");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

// Arranca el i2c
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(onRequest);

  Serial.println("=== Slave I2C listo. Comandos: LED_ON | LED_OFF ===");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    parseCommand(cmd);
  }
}