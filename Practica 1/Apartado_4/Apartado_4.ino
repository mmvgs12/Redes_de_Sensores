
#include "timers.h"
#include "mbed.h"

const int ADC_PIN = A0;
const int PWM_PIN = 3;

volatile bool readFlag = false; // flag de la interrupcion del timer

BBTimer adcTimer(BB_TIMER1); //timer para el adc

mbed::PwmOut pwm(digitalPinToPinName(PWM_PIN)); // declaracion del pwm

//Interrupcion del timer
void onTimer() {
  readFlag = true;
}
//Funcion para mandar el dato leido por el ADC
void sendADC() {
  int   raw     = analogRead(ADC_PIN);
  float voltage = raw * 3.3f / 4095.0f;
  
  char buf[64];
  sprintf(buf, "ADC: %4d  |  %.3f V", raw, voltage);
  Serial.println(buf);
}

//Esta es la funcion princpal, la que decide en funcion del mensaje que se le manda por el Serial
void parseCommand(String cmd) {
  cmd.trim();


  // ADC
  if (cmd == "ADC") {
    sendADC();
    return;
  }

  // ADC(x)
  if (cmd.startsWith("ADC(") && cmd.endsWith(")")) {
    int x = cmd.substring(4, cmd.length() - 1).toInt();

    if (x == 0) {
      // Si el numero que llega es 0, detiene el timer y el envio del ADC
      adcTimer.timerStop();
      Serial.println("INFO: envio periodico detenido");
    } else {
      //Configura el timer para mandar cada x segundos
      adcTimer.setupTimer((unsigned int)x * 1000000UL, onTimer);
      adcTimer.timerStart();
      char buf[48];
      sprintf(buf, "INFO: envio cada %d s activado", x);
      Serial.println(buf);
    }
    return;
  }

  // PWM(x)
  if (cmd.startsWith("PWM(") && cmd.endsWith(")")) {
    int x = cmd.substring(4, cmd.length() - 1).toInt();

    if (x < 0 || x > 9) {
      Serial.println("ERROR: PWM(x) acepta x entre 0 y 9");
      return;
    }

    //convierte el valor mandado por serial a una escala entre 0 y 1 y activa el duty
    float duty = x / 9.0f;
    pwm.write(duty);

    char buf[48];
    sprintf(buf, "INFO: PWM duty = %d/9 (%.1f%%)", x, duty * 100.0f);
    Serial.println(buf);
    return;
  }

  Serial.println("ERROR: comando desconocido. Usa ADC | ADC(x) | PWM(x)");
}

// --- Setup / Loop --------------------------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial);

  analogReadResolution(12);

  pwm.period(1.0f / 5000.0f);  //para 5 kHz se necesita periodo de 200 µs
  pwm.write(0.0f); //Duty 0% inicial

  //Declarancion del timer
  adcTimer.setupTimer(1000000UL, onTimer);

}

void loop() {

  if (readFlag) {
    readFlag = false;
    sendADC();
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    parseCommand(cmd);
  }
}