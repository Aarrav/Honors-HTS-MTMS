#include <Arduino.h>

const int encoderPin = 18;
const int pulsesPerRev = 20; 

volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile bool newPulse = false;

// Timeout threshold: if no pulse for 500ms, assume motor is stopped
const unsigned long TIMEOUT = 500000; 

void handlePulse() {
  unsigned long currentTime = micros();
  pulseInterval = currentTime - lastPulseTime;
  lastPulseTime = currentTime;
  newPulse = true;
}

void setup(){
  // Motor pins setup
  for(int i = 2; i <= 9; i++) pinMode(i, OUTPUT);

  digitalWrite(2, HIGH); 
  digitalWrite(4, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(8, HIGH);

  pinMode(encoderPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPin), handlePulse, RISING);

  Serial.begin(115200);
}

void loop(){
  // Motor control
  analogWrite(3, 200); 
  analogWrite(5, 200); 
  analogWrite(7, 200); 
  analogWrite(9, 200);

  unsigned long currentMillis = micros();
  
  // Check for motor timeout
  if (currentMillis - lastPulseTime > TIMEOUT) {
      Serial.println("Speed: 0 RPM");
      lastPulseTime = currentMillis; // Reset timer to prevent flooding
      delay(200); // Small delay to keep Serial readable
  } 
  else if (newPulse) {
    unsigned long intervalCopy;
    
    // Protect variables accessed by interrupts
    noInterrupts();
    intervalCopy = pulseInterval;
    newPulse = false;
    interrupts();
    
    float rpm = 60000000.0 / (intervalCopy * pulsesPerRev);
    
    Serial.print("Speed: ");
    Serial.print(rpm);
    Serial.println(" RPM");
  }
}