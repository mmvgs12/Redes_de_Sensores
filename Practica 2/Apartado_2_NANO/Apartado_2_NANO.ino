#include <Wire.h>

// Direccion I2C del Nano como esclavo
#define I2C_SLAVE_ADDR 0x55

volatile float ax = 0, ay = 0, az = 0;


// El LSM9DS1 está conectado al bus I2C interno del Nano (Wire1), no a los pines A4/A5
#define LSM_ADDR  0x6B  // Dirección I2C del acelerómetro/giroscopio
#define CTRL_REG6 0x20  // Registro de control: configura ODR y rango
#define OUT_X_L   0x28  // Primer registro de salida de aceleración (X low byte)

// Inicializa el LSM9DS1 por Wire1 (bus interno del Nano)
void imuInit() {
  Wire1.begin();
  Wire1.beginTransmission(LSM_ADDR);
  Wire1.write(CTRL_REG6);
  Wire1.write(0x60); // CTRL_REG6_XL = 0x60 → ODR 119Hz, rango ±2g, ancho de banda automático
  Wire1.endTransmission();
}
/*
Lee los 6 bytes de aceleración del LSM9DS1 (2 bytes por eje: X, Y, Z)
El bit 0x80 en la dirección del registro activa el autoincremento de dirección,
lo que permite leer los 6 bytes consecutivos en una sola transacción I2C
*/
//Funcion que lee los datos del IMU
void imuRead() {
  Wire1.beginTransmission(LSM_ADDR);
  Wire1.write(OUT_X_L | 0x80); // 0x80 = autoincremento de registro
  Wire1.endTransmission(false); // false = no liberar el bus (repeated start)
  Wire1.requestFrom(LSM_ADDR, 6);
  if (Wire1.available() < 6) return; // abortar si no hay datos suficientes

  // Reconstruir enteros de 16 bits: primero byte bajo, luego byte alto
  int16_t rx = Wire1.read() | (Wire1.read() << 8);
  int16_t ry = Wire1.read() | (Wire1.read() << 8);
  int16_t rz = Wire1.read() | (Wire1.read() << 8);

  // Sensibilidad del LSM9DS1 en modo ±2g: 0.061 mg/LSB
  // 0.061 mg/LSB × 0.00981
  const float scale = 0.061f * 0.00981f;
  ax = rx * scale;
  ay = ry * scale;
  az = rz * scale;
}

//Funcion que devuelve los datos cada vez que los pide el ESP32
void onRequest() {
  Wire.write((uint8_t*)&ax, 4); // ax: 4 bytes (float)
  Wire.write((uint8_t*)&ay, 4); // ay: 4 bytes (float)
  Wire.write((uint8_t*)&az, 4); // az: 4 bytes (float)
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // esperar a que el monitor serie esté listo (necesario en Nano 33 BLE)

  // Iniciar Wire como esclavo I2C ANTES que Wire1/IMU
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(onRequest); // registrar callback para peticiones del maestro
  Serial.println("I2C esclavo OK en 0x55");

  // Iniciar IMU en bus interno Wire1
  imuInit();
  Serial.println("IMU OK");

  Serial.println("Nano listo");
}

void loop() {
  // Muestrear el IMU continuamente cada 10ms (~100Hz)
  // Los datos quedan en ax, ay, az listos para ser enviados en onRequest()
  imuRead();
  delay(10);
}