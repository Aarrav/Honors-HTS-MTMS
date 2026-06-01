#include "servos.h"

// ============================================================
//  servos.cpp  —  Morph servo implementation
//
//  NOTE: morphUp() and morphDown() are BLOCKING.
//  While morphing, the main loop pauses — motors must be
//  stopped before calling these (handled in mavlink_handler).
// ============================================================

// Servo objects (one per corner)
static Servo servoFL;
static Servo servoFR;
static Servo servoBL;
static Servo servoBR;

// Track current morph position so we only move the delta needed
static int currentPos = 0;   // 0 = down, 105 = up

// Write all four servos for a given morph position (0..105)
static void applyMorphServos(int pos) {
    pos = constrain(pos, 0, 105);
    servoFL.write(5  + pos);
    servoFR.write(170 - pos);
    servoBL.write(160 - pos);
    servoBR.write(10  + pos);
}

// --- Public functions ---

void servosInit() {
    servoFL.attach(20);
    servoFR.attach(21);
    servoBL.attach(22);
    servoBR.attach(23);

    currentPos = 0;
    applyMorphServos(currentPos);   // start in down position
}

// Move from current position up to pos=95 (fully up).
// speed: 0 = slowest (100 ms/step), 99 = fastest (1 ms/step)
void morphUp(int speed) {
    speed = constrain(speed, 0, 99);
    int delayMs = 100 - speed;

    Serial.printf("[Morph] UP  start pos=%d\n", currentPos);

    for (int pos = currentPos; pos <= 105; pos++) {
        applyMorphServos(pos);
        currentPos = pos;
        delay(delayMs);
    }

    Serial.println("[Morph] UP  complete");
}

// Move from current position down to pos=0 (fully down).
void morphDown(int speed) {
    speed = constrain(speed, 0, 99);
    int delayMs = 100 - speed;

    Serial.printf("[Morph] DOWN start pos=%d\n", currentPos);

    for (int pos = currentPos; pos >= 0; pos--) {
        applyMorphServos(pos);
        currentPos = pos;
        delay(delayMs);
    }

    Serial.println("[Morph] DOWN complete");
}
