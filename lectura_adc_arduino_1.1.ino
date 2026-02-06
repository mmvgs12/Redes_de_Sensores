int ADC = A0;
int lectura_ADC = 0;
float valor_V = 0.0;
char buffer[30];

void setup() {

Serial.begin(19200);

analogReadResolution(12);

}

void loop() {

lectura_ADC = analogRead(ADC);
valor_V = lectura_ADC *(3.3 / 4095);

sprintf(buffer, "La tension de entrada es: %v", valor_V);
Serial.println(valor_V);
delay(1000);

}
