
#include "timers.h"

const int ADC_PIN = A0;
volatile bool readFlag = false; //flag de la interrupcion del timer
int raw = 0;
float voltage = 0.0;
char buf[64];

// Instancia global: el constructor no hace llamadas al sistema,
// por lo que es seguro declararlo en ámbito global.
BBTimer adcTimer(BB_TIMER1);

//interrupcion del timer
void onTimer() {
  readFlag = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  analogReadResolution(12);
  //Crea e inicializa el timer
  // BBTimer usa prescaler=4 → 1 tick = 1 µs
  // 10 s = 10 000 000 µs
  adcTimer.setupTimer(10000000UL, onTimer);
  adcTimer.timerStart();

}

void loop() {
  //Si ha saltado la interrupcion del timer, lee y manda datos
  if (readFlag) {
    readFlag = false;

    raw = analogRead(ADC_PIN);
    voltage = raw * 3.3f / 4095.0f;

    sprintf(buf, "[TIMER ISR] ADC: %4d  |  %.3f V", raw, voltage);
    Serial.println(buf);
  }
}