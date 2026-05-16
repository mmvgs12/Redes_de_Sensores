
#include "mbed.h"

const int ADC_PIN = A0;
const int PWM_PIN = 3;    // D3 = p. ej. PB_2 en Nano 33 BLE

int   raw     = 0;
float voltage = 0.0;
char  buf[64];


// PWM via mbed: periodo en segundos, duty como float 0.0-1.0
mbed::PwmOut pwm(digitalPinToPinName(PWM_PIN));

void setup() {
  Serial.begin(115200);
  while (!Serial);

  analogReadResolution(12);
  // Inicializacion del PWM
  pwm.period(1.0f / 5000.0f);  //para 5 kHz se necesita periodo de 200 µs
  pwm.write(0.0f);              // duty 0% inicial

}

void loop() {
  //lectura del pwm
  raw     = analogRead(ADC_PIN);
  voltage = raw * 3.3f / 4095.0f;

  // Escalado a 12 bits (0-4095) para el duty
  float duty = raw / 4095.0f;
  pwm.write(duty);

  //Imprime el porcentaje del duty 
  sprintf(buf, "ADC: %4d  |  %.3f V  |  duty: %.1f%%", raw, voltage, duty * 100.0f);
  Serial.println(buf);

  delay(100);
}