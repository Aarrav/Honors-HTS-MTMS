#include <Arduino.h>
#include <Servo.h>

Servo myServoFL;
Servo myServoFR;
Servo myServoBL;
Servo myServoBR;

void setup() {
  myServoFL.attach(23);
  myServoFR.attach(14);
  myServoBL.attach(0);
  myServoBR.attach(12);

}

void loop() {

  for(int pos = 0; pos <= 90; pos += 1) {
    myServoFL.write(pos);
    myServoFR.write(90-pos); 
    myServoBR.write(pos+10); 
    myServoBL.write((90-pos)+15);             
    delay(25);                       
  }
  delay(4000);

  for(int pos = 90; pos >= 0; pos -= 1) {
  myServoFL.write(pos);
  myServoFR.write(90 - pos); 
  myServoBR.write(pos + 10); 
  myServoBL.write((90 - pos) + 15);             
  delay(25); 
  } 

  delay(4000);                     
}  
  
