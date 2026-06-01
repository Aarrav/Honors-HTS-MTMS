#pragma once
#include <Arduino.h>

// ============================================================
//  mavlink_handler.h  —  MAVLink communication with Pixhawk 6C
//
//  Physical connection:
//    Pixhawk TELEM1  →  Teensy 4.0 Serial4 (RX4 pin 16, TX4 pin 17)
//    Baud rate: 57600
//
//  What this module does:
//    - Sends a heartbeat to the Pixhawk every second
//      (required so the Pixhawk knows a GCS is connected)
//    - After the first Pixhawk heartbeat is received,
//      requests RC_CHANNELS messages at 10 Hz
//    - Parses incoming RC_CHANNELS and:
//        CH1 = steering   (passed to driveFromRC)
//        CH2 = throttle   (passed to driveFromRC)
//        CH5 = morph      (>1500 = up,  <=1500 = down)
//        CH8 = arm/disarm (logged only)
//    - Stops motors + morphs before blocking morph move
//    - Implements a failsafe: motors stop if no RC for 400 ms
// ============================================================

void mavlinkInit();   // call once in setup()
void mavlinkUpdate(); // call every loop iteration
