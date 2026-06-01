#pragma once
#include <Arduino.h>

// ============================================================
//  motors.h  —  4x DC motor control (DIR + PWM per motor)
//
//  Wiring (Teensy 4.0 pins):
//    Motor    DIR pin   PWM pin
//    FL        4         5
//    FR        7         8
//    BL        1         2
//    BR       10        11
//
//  Driver behaviour: PWM is INVERTED
//    analogWrite(pwmPin, 255) = stopped
//    analogWrite(pwmPin,   0) = full speed
// ============================================================

// --- Pin definitions ---
constexpr uint8_t M_FL_DIR = 4,  M_FL_PWM = 5;
constexpr uint8_t M_FR_DIR = 7,  M_FR_PWM = 8;
constexpr uint8_t M_BL_DIR = 1,  M_BL_PWM = 2;
constexpr uint8_t M_BR_DIR = 10, M_BR_PWM = 11;

// --- RC / PWM tuning ---
constexpr int RC_CENTER      = 1500;  // neutral stick value (µs)
constexpr int RC_DEADBAND    = 40;    // ignore stick movement smaller than this
constexpr int PWM_MAX        = 255;
constexpr int PWM_MIN_START  = 25;    // minimum PWM to overcome stiction (set 0 to disable)

// --- Public functions ---
void motorsInit();          // call once in setup()
void stopAllMotors();       // immediately stop all four motors
void driveFromRC(uint16_t ch1_steer, uint16_t ch2_throttle);  // arcade-mix drive
