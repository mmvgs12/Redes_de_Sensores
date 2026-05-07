#include <ArduinoBLE.h>

const char* MY_UUID    = "19B10000-5365-7267-696F-000000000001"; //UUID para serivicio generico con mi nombre 5365-7267-696F
const char* ADVERTISING_NAME = "NanoIoT-Sergio_Muñoz"; //Advertising del dispositivo
 
BLEService advertService(MY_UUID);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("ERROR: no se pudo iniciar BLE"); //Si falla la conexion al BLE, "para" el programa
    while(1);
  }
  //Genera el adviertising con los servicios y caracteristicas para poder verlo desde nrfConnect
  BLE.setLocalName(ADVERTISING_NAME);
  BLE.setAdvertisedService(advertService);
  BLE.addService(advertService);
  BLE.advertise();

  Serial.print("Advertising BLE como: ");
  Serial.println(ADVERTISING_NAME);

}

void loop() {
  BLE.poll(); // necesario para procesar eventos BLE
  delay(200);
}