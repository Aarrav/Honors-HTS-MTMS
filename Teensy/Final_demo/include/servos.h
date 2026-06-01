#pragma once
#include <Arduino.h>
#include <Servo.h>

// ============================================================
//  servos.h  —  4x morph servo control
//
//  Wiring (Teensy 4.0 pins):
//    Servo    Pin
//    FL        20
//    FR        21
//    BL        22
//    BR        23
//
//  Morph range: pos 0 (down) … 95 (up)
//  Each servo gets a fixed offset so all four corners move together:
//    FL = 20 + pos        FR = 170 - pos
//    BL = 160 - pos       BR = 10  + pos
// ============================================================

// --- Morph speed ---
// speedMorph 0  →  slowest (100 ms per step)
// speedMorph 99 →  fastest (1 ms per step)
// Recommended range: 10 – 50
constexpr int DEFAULT_MORPH_SPEED = 20;

// --- Public functions ---
void servosInit();                          // call once in setup()
void morphUp(int speed = DEFAULT_MORPH_SPEED);    // blocking: move to fully-up position
void morphDown(int speed = DEFAULT_MORPH_SPEED);  // blocking: move to fully-down position
