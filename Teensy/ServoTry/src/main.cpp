#include <Arduino.h>
#include <Servo.h>

Servo myServoFL;
Servo myServoFR;
Servo myServoBL;
Servo myServoBR;

void setup() {
  myServoFL.attach(20);
  myServoFR.attach(21);
  myServoBR.attach(22);
  myServoBL.attach(23);

}

void morphUp(int speedMorphUp) {
  for(int pos = 0; pos <= 90; pos += 1) {
    myServoFL.write(pos+7); //10 to 100
    myServoFR.write(190-pos); //190 to 100
    myServoBR.write(pos+25); //170 to 85
    myServoBL.write(180-pos); //25 to 115            
    
    delay(100 - speedMorphUp);  //if speed == 0, delay is 100, if speed == 100, delay is 0             
  }
  return;   
}

void morphDown(int speedMorphDown) {
  for(int pos = 90; pos >= 0; pos -= 1) {
    myServoFL.write(pos+7); //100 to 10
    myServoFR.write(190-pos); //100 to 190
    myServoBR.write(pos+25); //85 to 170
    myServoBL.write(180-pos); //115 to 25            
    
    delay(100 - speedMorphDown);  //if speed == 0, delay is 100, if speed == 100, delay is 0             
  }
  return;   
}

void loop() {

  // for(int pos = 0; pos <= 90; pos += 1) {
  //   myServoFL.write(pos);
  //   //myServoFR.write(90-pos); 
  //   //myServoBR.write(pos+10); 
  //   //myServoBL.write((90-pos)+15);             
  //   delay(25);                       
  // }
  // delay(4000);   
  
  //myServoFL.write(5); //100 to 5
  //myServoFR.write(190); //100 to 190
  //myServoBL.write(25); //115 to 25
  //myServoBR.write(180); //85 to 170

  morphDown(20);
  delay(4000);
  morphUp(20);
  delay(4000);
}  
  
