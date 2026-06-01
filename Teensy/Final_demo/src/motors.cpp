#include "motors.h"

// ============================================================
//  motors.cpp  —  DC motor implementation
// ============================================================

// --- Internal helpers (not exposed in the header) ---

// Remove small stick jitter around zero
static int applyDeadband(int value, int deadband) {
    return (abs(value) < deadband) ? 0 : value;
}

// Convert RC pulse (1000–2000 µs) to motor command (-255 … +255)
static int rcToCmd(uint16_t rcValue) {
    int v = (int)rcValue - RC_CENTER;       // centre around 0  →  -500 … +500
    v = applyDeadband(v, RC_DEADBAND);
    v = constrain(v, -500, 500);
    return (int)map(v, -500, 500, -PWM_MAX, PWM_MAX);
}

// Drive a single motor.
//   cmd > 0  →  forward
//   cmd < 0  →  reverse
//   cmd = 0  →  stop
// PWM is INVERTED on this driver board (255 = stop, 0 = full speed).
static void setMotor(uint8_t dirPin, uint8_t pwmPin, int cmd) {
    cmd = constrain(cmd, -PWM_MAX, PWM_MAX);

    if (cmd == 0) {
        analogWrite(pwmPin, 255);   // inverted: 255 = stopped
        return;
    }

    bool forward = (cmd > 0);
    int  mag     = abs(cmd);

    // Apply minimum start threshold to overcome static friction
    if (mag > 0 && mag < PWM_MIN_START) mag = PWM_MIN_START;

    digitalWrite(dirPin, forward ? HIGH : LOW);
    analogWrite(pwmPin, 255 - mag);   // inverted: 255-mag maps speed correctly
}

// --- Public functions ---

void motorsInit() {
    pinMode(M_FL_DIR, OUTPUT);  pinMode(M_FL_PWM, OUTPUT);
    pinMode(M_FR_DIR, OUTPUT);  pinMode(M_FR_PWM, OUTPUT);
    pinMode(M_BL_DIR, OUTPUT);  pinMode(M_BL_PWM, OUTPUT);
    pinMode(M_BR_DIR, OUTPUT);  pinMode(M_BR_PWM, OUTPUT);

    analogWriteResolution(8);   // 8-bit PWM  →  0..255

    stopAllMotors();
}

void stopAllMotors() {
    setMotor(M_FL_DIR, M_FL_PWM, 0);
    setMotor(M_FR_DIR, M_FR_PWM, 0);
    setMotor(M_BL_DIR, M_BL_PWM, 0);
    setMotor(M_BR_DIR, M_BR_PWM, 0);
}

// Arcade-mix drive:
//   Left  side command = throttle + steer
//   Right side command = throttle - steer
// This gives smooth, proportional turning while moving forward/backward.
void driveFromRC(uint16_t ch1_steer, uint16_t ch2_throttle) {
    int steer    = rcToCmd(ch1_steer);     // negative = left,    positive = right
    int throttle = rcToCmd(ch2_throttle);  // negative = reverse, positive = forward

    int leftCmd  = constrain(throttle + steer, -PWM_MAX, PWM_MAX);
    int rightCmd = constrain(throttle - steer, -PWM_MAX, PWM_MAX);

    // Left side motors (FL + BL) — negated to match physical orientation
    setMotor(M_FL_DIR, M_FL_PWM, -leftCmd);
    setMotor(M_BL_DIR, M_BL_PWM, -leftCmd);

    // Right side motors (FR + BR)
    setMotor(M_FR_DIR, M_FR_PWM,  rightCmd);
    setMotor(M_BR_DIR, M_BR_PWM,  rightCmd);
}
