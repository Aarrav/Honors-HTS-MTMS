#include <Arduino.h>

// RE-ASSIGNED PINS (Avoid 0 and 1 because of Serial)
#define DC_MOTOR_ENC_FR 0 // Moved from 0
#define DC_MOTOR_DIR_FR 1 
#define DC_MOTOR_PWM_FR 2 

#define DC_MOTOR_ENC_FL 3
#define DC_MOTOR_DIR_FL 4
#define DC_MOTOR_PWM_FL 5

#define DC_MOTOR_ENC_BR 6
#define DC_MOTOR_DIR_BR 7
#define DC_MOTOR_PWM_BR 8

#define DC_MOTOR_ENC_BL 9
#define DC_MOTOR_DIR_BL 10
#define DC_MOTOR_PWM_BL 11

// Variables for velocity calculation
volatile long countFR = 0, countFL = 0, countBR = 0, countBL = 0;
long prevFR = 0, prevFL = 0, prevBR = 0, prevBL = 0;
unsigned long lastTime = 0;
const int interval = 10; 

// ISR Functions
void isrFR() { countFR++; }
void isrFL() { countFL++; }
void isrBR() { countBR++; }
void isrBL() { countBL++; }

void setup() {
  Serial.begin(115200);

  pinMode(DC_MOTOR_ENC_FR, INPUT_PULLUP);
  pinMode(DC_MOTOR_ENC_FL, INPUT_PULLUP);
  pinMode(DC_MOTOR_ENC_BR, INPUT_PULLUP);
  pinMode(DC_MOTOR_ENC_BL, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(DC_MOTOR_ENC_FR), isrFR, RISING);
  attachInterrupt(digitalPinToInterrupt(DC_MOTOR_ENC_FL), isrFL, RISING);
  attachInterrupt(digitalPinToInterrupt(DC_MOTOR_ENC_BR), isrBR, RISING);
  attachInterrupt(digitalPinToInterrupt(DC_MOTOR_ENC_BL), isrBL, RISING);

  pinMode(DC_MOTOR_DIR_FR, OUTPUT); pinMode(DC_MOTOR_PWM_FR, OUTPUT);
  pinMode(DC_MOTOR_DIR_FL, OUTPUT); pinMode(DC_MOTOR_PWM_FL, OUTPUT);
  pinMode(DC_MOTOR_DIR_BR, OUTPUT); pinMode(DC_MOTOR_PWM_BR, OUTPUT);
  pinMode(DC_MOTOR_DIR_BL, OUTPUT); pinMode(DC_MOTOR_PWM_BL, OUTPUT);

  digitalWrite(DC_MOTOR_DIR_FR, HIGH);
  digitalWrite(DC_MOTOR_DIR_FL, HIGH);
  digitalWrite(DC_MOTOR_DIR_BR, HIGH);
  digitalWrite(DC_MOTOR_DIR_BL, HIGH);
}

void moveFront(float targetVel, float Kp) {
  if (millis() - lastTime >= interval) {
    lastTime = millis();

    // 1. Snapshot counts
    noInterrupts();
    long snapFR = countFR; long snapFL = countFL;
    long snapBR = countBR; long snapBL = countBL;
    interrupts();

    // 2. Calculate Actual Velocity
    float actFR = snapFR - prevFR;
    float actFL = snapFL - prevFL;
    float actBR = snapBR - prevBR;
    float actBL = snapBL - prevBL;

    prevFR = snapFR; prevFL = snapFL;
    prevBR = snapBR; prevBL = snapBL;

    // 3. Calculate Errors
    float errorFR = targetVel - actFR;
    float errorFL = targetVel - actFL;
    float errorBR = targetVel - actBR;
    float errorBL = targetVel - actBL;

    // 4. BASIC P-CONTROLLER MATH
    // PWM is directly proportional to the error
    // We add a "Feed Forward" of targetVel*2 so the motor has a base power
    float pwmFR = 255 - ((targetVel * 2) + (errorFR * Kp));
    float pwmFL = 255 - ((targetVel * 2) + (errorFL * Kp));
    float pwmBR = 255 - ((targetVel * 2) + (errorBR * Kp*1.2));
    float pwmBL = 255 - ((targetVel * 4) + (errorBL * Kp*1.2));

    // 5. Constrain and Write
    analogWrite(DC_MOTOR_PWM_FR, constrain((int)pwmFR, 0, 255));
    analogWrite(DC_MOTOR_PWM_FL, constrain((int)pwmFL, 0, 255));
    analogWrite(DC_MOTOR_PWM_BR, constrain((int)pwmBR, 0, 255));
    analogWrite(DC_MOTOR_PWM_BL, constrain((int)pwmBL, 0, 255));

    // 6. Print (Target, FR, FL, BR, BL)
    Serial.print(targetVel); Serial.print("\t"); 
    Serial.print(actFR);    Serial.print("\t");
    Serial.print(actFL);    Serial.print("\t");
    Serial.print(actBR);    Serial.print("\t");
    Serial.print(actBL);    Serial.print("\t");
    Serial.print(errorFR);   Serial.print("\t");
    Serial.print(errorFL);   Serial.print("\t");
    Serial.print(errorBR);   Serial.print("\t");
    Serial.print(errorBL);   Serial.print("\t");
    Serial.print(pwmFR);    Serial.print("\t");
    Serial.print(pwmFL);    Serial.print("\t");
    Serial.print(pwmBR);    Serial.print("\t");
    Serial.println(pwmBL);
  }
}

void loop() {
  // Start with a small Kp like 2.0 or 5.0
  moveFront(1.5, 35.0); 
}