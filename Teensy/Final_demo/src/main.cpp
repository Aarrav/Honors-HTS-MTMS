#include <Arduino.h>
#include "motors.h"
#include "servos.h"
#include "mavlink_handler.h"

// ============================================================
//  main.cpp  —  Entry point
//
//  This file is intentionally kept minimal.
//  All logic lives in the three modules:
//    motors.cpp         —  DC motor arcade-mix drive
//    servos.cpp         —  Blocking morph servo control
//    mavlink_handler.cpp — Pixhawk heartbeat + RC parsing
// ============================================================

void setup() {
    Serial.begin(115200);

    // Wait up to 2 s for the USB serial monitor (optional, remove if unneeded)
    uint32_t t0 = millis();
    while (!Serial && millis() - t0 < 2000) {}

    Serial.println("==============================================");
    Serial.println(" MorphBot — Teensy 4.0");
    Serial.println(" Motors  : FL(4,5) FR(7,8) BL(1,2) BR(10,11)");
    Serial.println(" Servos  : FL(20) FR(21) BL(22) BR(23)");
    Serial.println(" Pixhawk : Serial4 RX=16 TX=17 @ 57600 baud");
    Serial.println("==============================================");

    motorsInit();       // configure motor pins, stop all motors
    servosInit();       // attach servos, move to down position
    mavlinkInit();      // open Serial4, wait for Pixhawk heartbeat
}

void loop() {
    // mavlinkUpdate() handles:
    //   - sending our heartbeat every 1 s
    //   - parsing incoming RC_CHANNELS
    //   - calling driveFromRC() for CH1/CH2
    //   - calling morphUp() / morphDown() for CH5
    //   - stopping motors on RC timeout (failsafe)
    mavlinkUpdate();
}
