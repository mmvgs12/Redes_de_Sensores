int ADC = A0;
int ADC_resolution = 12;

void setup() {

Serial.begin(19200);

analogReadResolution(ADC_resolution);

}

void loop() {

sprintf(buffer, "La tension de entrada es: %v", valor_V);
Serial.println(valor_V);
delay(1000);

}

int leerADC(float Vmax, int resolution){

  lectura_ADC = analogRead(ADC);
  return lectura_ADC *(Vmax / ((2^resolution)-1));

}
