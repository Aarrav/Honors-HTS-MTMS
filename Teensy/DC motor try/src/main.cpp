#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(8, HIGH);

}
void loop() {
  // put your main code here, to run repeatedly:
  

  analogWrite(5, 250);
  delay(1000);
  analogWrite(5, 0);
  delay(1000);
}

/*
int i = 0;
unsigned long time = 0;
bool flag = HIGH;
void setup() {
// put your setup code here, to run once:
Serial.begin(115200);
pinMode(10, OUTPUT); //PWM PIN 10 with PWM wire
pinMode(11, OUTPUT);//direction control PIN 11 with direction wire
}
void loop() {
// put your main code here, to run repeatedly:
if (millis() - time > 5000) {
flag = !flag;
digitalWrite(10, flag);
time = millis();
}
if (Serial.available()) {
analogWrite(11, Serial.parseInt()); //input speed (must be int)
delay(200);
}
for(int j = 0;j<8;j++) {
i += pulseIn(9, HIGH, 500000); //SIGNAL OUTPUT PIN 9 with white line,cycle = 2*i,1s = 1000000us，Signa
l cycle pulse number：27*2
}
i = i >> 3;
Serial.print(111111 / i); //speed r/min (60*1000000/(45*6*2*i))
Serial.println(" r/min");
i = 0;
}
*/