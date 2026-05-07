#include <ArduinoBLE.h>

// IDs del LED RGB en Nano 33 BLE Sense
const int LED_R = LEDR;
const int LED_G = LEDG;
const int LED_B = LEDB;

const char* ADVERTISING_NAME = "NanoLED_S_M"; //Advertising del dispositivo
const char* BLE_SERVICE = "180A"; // Corresponde con el UUID del servicio Device Information
const char* BLE_CHARACTERISTIC = "2A57"; // Corresponde con el UUID de la caracteristica Digital Output

BLEService ledService(BLE_SERVICE); // Crea el servicio del BLE
BLEStringCharacteristic colorChar(BLE_CHARACTERISTIC, BLEWrite, 4); //Crea la caracteristica dentro del servicio,como escritura de char y con 4 bytes 


void setup() {

  Serial.begin(115200);

  pinMode(LED_R, OUTPUT); pinMode(LED_G, OUTPUT); pinMode(LED_B, OUTPUT); //Configura los pines del arduino que van a los leds como salida


  digitalWrite(LED_R, HIGH); digitalWrite(LED_G, HIGH); digitalWrite(LED_B, HIGH); // Apaga los leds, activo a nivel bajo

  if (!BLE.begin()) 
  { 
    Serial.println("ERROR BLE"); 
    while(1); //Si falla la conexion al BLE, "para" el programa
  }

  //Genera el adviertising con los servicios y caracteristicas para poder verlo desde nrfConnect
  BLE.setLocalName(ADVERTISING_NAME); 
  BLE.setAdvertisedService(ledService);
  ledService.addCharacteristic(colorChar);
  BLE.addService(ledService);
  BLE.advertise();

  Serial.println("BLE LED control listo. Escribe R/G/B/0 desde nRF Connect.");
}

void loop() {
  // comprueba si hay algun dispositivo conectado a arduino
  BLEDevice central = BLE.central();
  //Si esta conectado y mientras siga asi, comprueba si en la caracteristica colorChar se ha escrito algun char, si es asi, comprueba que char se ha escrito y actua en consecuencia.
  if (central) {
    while (central.connected()) { 
      if (colorChar.written()) {
        String val = colorChar.value();
        val.toUpperCase();
        // Apagar todo
        
        if (val == "0") {
          digitalWrite(LED_R, HIGH);
          digitalWrite(LED_G, HIGH); 
          digitalWrite(LED_B, HIGH);
        } else if (val == "R") {
          digitalWrite(LED_R, LOW);
          digitalWrite(LED_G, HIGH); 
          digitalWrite(LED_B, HIGH);
        } else if (val == "G") {
          digitalWrite(LED_R, HIGH);
          digitalWrite(LED_G, LOW); 
          digitalWrite(LED_B, HIGH);
        } else if (val == "B") {
          digitalWrite(LED_R, HIGH);
          digitalWrite(LED_G, HIGH); 
          digitalWrite(LED_B, LOW);
        }
      }
    }
    Serial.println("Central desconectada.");
  }
}