// ============================================================
//  TEST 1: Servo / Morph Test
//  
//  PURPOSE: Verify all 4 servos are wired correctly and move
//           in the right direction before touching motors or MAVLink.
//
//  WHAT YOU NEED:
//    - Teensy 4.0 + servo board only
//    - USB connected to PC for Serial monitor
//    - NO Pixhawk, NO motors needed
//
//  HOW TO USE:
//    1. Upload this sketch
//    2. Open Serial Monitor at 115200 baud
//    3. Type a command and press Enter:
//         u  →  morph UP   (slow, so you can watch each servo)
//         d  →  morph DOWN (slow)
//         0  →  jump straight to DOWN position
//         9  →  jump straight to UP position
//         p  →  print current servo angles
//
//  WHAT TO CHECK:
//    - All 4 servos move smoothly without grinding
//    - When morphing UP:   FL and BR rotate clockwise (increasing angle)
//                          FR and BL rotate counter-clockwise (decreasing angle)
//    - When morphing DOWN: opposite of above
//    - No servo hits a mechanical hard stop (grinding noise = stop immediately)
// ============================================================

#include <Arduino.h>
#include <Servo.h>

Servo servoFL, servoFR, servoBL, servoBR;

int currentPos = 0;   // 0 = down, 95 = up

// Write all four servos for a given morph position
void applyServos(int pos) {
    pos = constrain(pos, 0, 100);
    servoFL.write(5  + pos);
    servoFR.write(170 - pos);
    servoBL.write(145 - pos);
    servoBR.write(25  + pos);
}

void printAngles() {
    int pos = currentPos;
    Serial.printf("  pos=%d  |  FL=%d  FR=%d  BL=%d  BR=%d\n",
                  pos, 20+pos, 170-pos, 160-pos, 10+pos);
}

void morphUp(int speedMs = 80) {
    Serial.println(">> Morphing UP...");
    for (int pos = currentPos; pos <= 95; pos++) {
        applyServos(pos);
        currentPos = pos;
        printAngles();
        delay(speedMs);
    }
    Serial.println(">> Done.");
}

void morphDown(int speedMs = 80) {
    Serial.println(">> Morphing DOWN...");
    for (int pos = currentPos; pos >= 0; pos--) {
        applyServos(pos);
        currentPos = pos;
        printAngles();
        delay(speedMs);
    }
    Serial.println(">> Done.");
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000) {}

    servoFL.attach(20);
    servoFR.attach(21);
    servoBL.attach(22);
    servoBR.attach(23);

    currentPos = 0;
    applyServos(0);

    Serial.println("===================================");
    Serial.println(" TEST 1: Servo / Morph Test");
    Serial.println("===================================");
    Serial.println("Commands (type + Enter):");
    Serial.println("  u  = morph UP   (slow)");
    Serial.println("  d  = morph DOWN (slow)");
    Serial.println("  0  = jump to DOWN position");
    Serial.println("  9  = jump to UP position");
    Serial.println("  p  = print current angles");
    Serial.println("-----------------------------------");
    Serial.println("Starting in DOWN position.");
    printAngles();
}

void loop() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();

        // Flush rest of line (handles Enter key)
        while (Serial.available()) Serial.read();

        switch (cmd) {
            case 'u':
                morphUp(80);    // 80 ms per step = slow enough to watch
                break;
            case 'd':
                morphDown(80);
                break;
            case '0':
                Serial.println(">> Jumping to DOWN position");
                applyServos(0);
                currentPos = 0;
                printAngles();
                break;
            case '9':
                Serial.println(">> Jumping to UP position");
                applyServos(95);
                currentPos = 95;
                printAngles();
                break;
            case 'p':
                printAngles();
                break;
            default:
                break;
        }
    }
}