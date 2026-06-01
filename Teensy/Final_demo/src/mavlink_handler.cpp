#include "mavlink_handler.h"
#include "motors.h"
#include "servos.h"

// MAVLink header — uses the common dialect from c_library_v2
#include <common/mavlink.h>

// ============================================================
//  mavlink_handler.cpp  —  MAVLink implementation
// ============================================================

// --- Serial port connected to Pixhawk TELEM1 ---
HardwareSerial &FC = Serial4;
constexpr uint32_t FC_BAUD = 57600;

// --- This device's MAVLink identity ---
// Poses as a GCS (ground control station) so Pixhawk responds normally
constexpr uint8_t MY_SYSID  = 255;
constexpr uint8_t MY_COMPID = MAV_COMP_ID_ONBOARD_COMPUTER;

// --- Pixhawk's MAVLink identity (filled in after first heartbeat) ---
static uint8_t target_sysid  = 1;
static uint8_t target_compid = 1;

// --- State flags ---
static bool rcStreamRequested = false;  // have we asked for RC_CHANNELS yet?
static bool morphIsUp         = false;  // tracks current morph position

// --- Timers ---
static elapsedMillis heartbeatTimer;  // sends heartbeat every 1000 ms
static elapsedMillis rcTimeoutTimer;  // failsafe: stop motors if RC goes quiet

constexpr uint16_t RC_TIMEOUT_MS = 400;  // ms without RC before motors stop

// ============================================================
//  Internal: send our heartbeat to the Pixhawk
// ============================================================
static void sendHeartbeat() {
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    mavlink_msg_heartbeat_pack(
        MY_SYSID, MY_COMPID, &msg,
        MAV_TYPE_GCS, MAV_AUTOPILOT_INVALID,
        MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 0, MAV_STATE_ACTIVE
    );

    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    FC.write(buf, len);
}

// ============================================================
//  Internal: ask Pixhawk to send RC_CHANNELS at 10 Hz
// ============================================================
static void requestRCStream() {
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    mavlink_msg_command_long_pack(
        MY_SYSID, MY_COMPID, &msg,
        target_sysid, target_compid,
        MAV_CMD_SET_MESSAGE_INTERVAL, 0,
        (float)MAVLINK_MSG_ID_RC_CHANNELS,  // which message to stream
        100000.0f,                           // interval in µs = 10 Hz
        0, 0, 0, 0, 0
    );

    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    FC.write(buf, len);
}

// ============================================================
//  Internal: debug helper — print direction arrow to Serial
// ============================================================
static void printDirection(uint16_t ch1, uint16_t ch2) {
    String dir = "";
    if      (ch2 > 1540) dir += "Forward ";
    else if (ch2 < 1460) dir += "Backward ";

    if      (ch1 > 1540) dir += "Right";
    else if (ch1 < 1460) dir += "Left";

    if (dir == "") dir = "Neutral";

    // Build arrow
    String arrow = "";
    if (dir.indexOf("Forward")  != -1) arrow += "↑";
    if (dir.indexOf("Backward") != -1) arrow += "↓";
    if (dir.indexOf("Left")     != -1) arrow += "←";
    if (dir.indexOf("Right")    != -1) arrow += "→";
    if (arrow == "") arrow = "•";

    Serial.printf("[RC] Direction: %-18s %s\n", dir.c_str(), arrow.c_str());
}

// ============================================================
//  Internal: handle an incoming RC_CHANNELS message
// ============================================================
static void handleRCChannels(const mavlink_message_t &msg) {
    mavlink_rc_channels_t rc;
    mavlink_msg_rc_channels_decode(&msg, &rc);

    // Reset failsafe timer — we got a fresh RC packet
    rcTimeoutTimer = 0;

    // --- CH8: Arm / Disarm (informational log only) ---
    Serial.println(rc.chan8_raw > 1500 ? "[RC] ARMED" : "[RC] DISARMED");

    // --- CH5: Morph up / down ---
    bool wantMorphUp = (rc.chan5_raw > 1500);

    if (wantMorphUp && !morphIsUp) {
        // Stop motors first — morph is blocking
        stopAllMotors();
        Serial.println("[Morph] Stopping motors, morphing UP...");
        morphUp(DEFAULT_MORPH_SPEED);
        morphIsUp = true;

    } else if (!wantMorphUp && morphIsUp) {
        // Stop motors first — morph is blocking
        stopAllMotors();
        Serial.println("[Morph] Stopping motors, morphing DOWN...");
        morphDown(DEFAULT_MORPH_SPEED);
        morphIsUp = false;
    }

    // --- CH1 + CH2: Drive motors (arcade mix) ---
    // Only drive if we are NOT in the middle of a morph transition
    // (morphUp/Down already returned at this point, so it is safe)
    driveFromRC(rc.chan1_raw, rc.chan2_raw);

    // --- Debug output ---
    printDirection(rc.chan1_raw, rc.chan2_raw);
    Serial.printf("[RC] CH1=%u  CH2=%u  CH5=%u  CH8=%u\n",
                  rc.chan1_raw, rc.chan2_raw, rc.chan5_raw, rc.chan8_raw);
    Serial.println("--------------------------------------");
}

// ============================================================
//  Public API
// ============================================================

void mavlinkInit() {
    FC.begin(FC_BAUD);
    heartbeatTimer = 0;
    rcTimeoutTimer = 0;
    Serial.println("[MAVLink] Serial4 opened at 57600 baud");
    Serial.println("[MAVLink] Waiting for Pixhawk heartbeat...");
}

void mavlinkUpdate() {
    // --- Failsafe: stop motors if RC packets stop arriving ---
    if (rcTimeoutTimer > RC_TIMEOUT_MS) {
        stopAllMotors();
    }

    // --- Send our heartbeat every second ---
    if (heartbeatTimer >= 1000) {
        heartbeatTimer = 0;
        sendHeartbeat();
    }

    // --- Parse every byte available on the FC serial port ---
    while (FC.available() > 0) {
        static mavlink_message_t msg;
        static mavlink_status_t  status;

        uint8_t c = FC.read();

        if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
            switch (msg.msgid) {

                // Pixhawk heartbeat → latch its address, request RC stream once
                case MAVLINK_MSG_ID_HEARTBEAT:
                    if (!rcStreamRequested) {
                        target_sysid  = msg.sysid;
                        target_compid = msg.compid;
                        requestRCStream();
                        rcStreamRequested = true;
                        Serial.printf("[MAVLink] Pixhawk found: sys=%u comp=%u — RC stream requested\n",
                                      target_sysid, target_compid);
                    }
                    break;

                // RC channel data → drive motors + morph
                case MAVLINK_MSG_ID_RC_CHANNELS:
                    handleRCChannels(msg);
                    break;

                default:
                    break;
            }
        }
    }
}
