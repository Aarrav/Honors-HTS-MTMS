#include <C:\Users\20232510\OneDrive - TU Eindhoven\Documents\Arduino\libraries\MAVLink\mavlink\common\mavlink.h>
#include <Arduino.h>
#include <Servo.h>

// -------------------------
// MAVLink / Serial Settings
// -------------------------
HardwareSerial &FC = Serial4;
const uint32_t FC_BAUD  = 57600;
const uint32_t USB_BAUD = 115200;

const uint8_t MY_SYSID  = 255;
const uint8_t MY_COMPID = MAV_COMP_ID_ONBOARD_COMPUTER;

uint8_t target_sysid  = 1;
uint8_t target_compid = 1;

elapsedMillis hbTimer;
bool requested_rc_stream = false;

// -------------------------
// Morph Servos
// -------------------------
Servo myServoFL;
Servo myServoFR;
Servo myServoBL;
Servo myServoBR;

const uint8_t SERVO_PIN_FL = 20;
const uint8_t SERVO_PIN_FR = 21;
const uint8_t SERVO_PIN_BR = 22;
const uint8_t SERVO_PIN_BL = 23;

int morphPos = 0;
int morphTarget = 0;
bool morphActive = false;
elapsedMillis morphStepTimer;
uint16_t morphStepIntervalMs = 10;

static inline int clampServoAngle(int angleDeg) {
  if (angleDeg < 0) return 0;
  if (angleDeg > 180) return 180;
  return angleDeg;
}

void applyMorphServos(int pos /*0..90*/) {
  int fl = clampServoAngle(pos + 7);
  int fr = clampServoAngle(190 - pos);
  int br = clampServoAngle(pos + 25);
  int bl = clampServoAngle(180 - pos);

  myServoFL.write(fl);
  myServoFR.write(fr);
  myServoBR.write(br);
  myServoBL.write(bl);
}

void updateMorphMotion() {
  if (!morphActive) return;
  if (morphStepTimer < morphStepIntervalMs) return;
  morphStepTimer = 0;

  if (morphPos < morphTarget) morphPos++;
  else if (morphPos > morphTarget) morphPos--;
  else { morphActive = false; return; }

  applyMorphServos(morphPos);
}

// -------------------------
// 4x DC MOTOR CONTROL (DIR + PWM per motor)
// -------------------------
// Pinout (blue = DIR, gray = PWM)
const uint8_t M_BL_DIR = 1,  M_BL_PWM = 2;    // BL  blue 1,  gray 2
const uint8_t M_BR_DIR = 10, M_BR_PWM = 11;   // BR  blue 10, gray 11
const uint8_t M_FL_DIR = 4,  M_FL_PWM = 5;    // FL  blue 4,  gray 5
const uint8_t M_FR_DIR = 7,  M_FR_PWM = 8;    // FR  blue 7,  gray 8

// RC interpretation
const int RC_CENTER   = 1500;
const int RC_DEADBAND = 40;      // prevents jitter near center

// PWM settings
const int PWM_MAX = 255;
const int PWM_MIN_START = 25;    // helps overcome stiction; set 0 if you dislike it

// Failsafe: stop motors if RC not received
elapsedMillis rcTimeout;
const uint16_t RC_TIMEOUT_MS = 400;

// Drive mode:
// 0 = arcade mix (proportional tank-like control, throttle + steer combine)
// 1 = rotate in place whenever steering is not neutral (ignores throttle while turning)
#define PURE_TANK_TURN 0

static inline int applyDeadband(int x, int deadband) {
  return (abs(x) < deadband) ? 0 : x;
}

// Convert RC 1000..2000 => -255..+255
int rcToCmd255(uint16_t rc) {
  int v = (int)rc - RC_CENTER;      // -500..+500
  v = applyDeadband(v, RC_DEADBAND);
  v = constrain(v, -500, 500);
  long out = map(v, -500, 500, -PWM_MAX, PWM_MAX);
  return (int)out;
}

// One motor: DIR sets sign, PWM is INVERTED on your hardware (0 = max speed, 255 = stop)
// Assumes DIR HIGH = forward, DIR LOW = reverse.
void setMotor(uint8_t dirPin, uint8_t pwmPin, int cmd /*-255..+255*/) {
  cmd = constrain(cmd, -PWM_MAX, PWM_MAX);

  // Stop = PWM 255 (per your driver behavior)
  if (cmd == 0) {
    analogWrite(pwmPin, 255);
    return;
  }

  bool forward = (cmd > 0);
  int mag = abs(cmd);

  if (mag > 0 && mag < PWM_MIN_START) mag = PWM_MIN_START;

  digitalWrite(dirPin, forward ? HIGH : LOW);

  // Inverted PWM: mag 0..255 -> pwmOut 255..0
  int pwmOut = 255 - mag;
  pwmOut = constrain(pwmOut, 0, 255);
  analogWrite(pwmPin, pwmOut);
}

void stopAllMotors() {
  setMotor(M_FL_DIR, M_FL_PWM, 0);
  setMotor(M_FR_DIR, M_FR_PWM, 0);
  setMotor(M_BL_DIR, M_BL_PWM, 0);
  setMotor(M_BR_DIR, M_BR_PWM, 0);
}

// Drive logic using CH1 steering, CH2 throttle
void driveFromRC(uint16_t ch1_steer, uint16_t ch2_throttle) {
  int steer    = rcToCmd255(ch1_steer);      // -255 left ... +255 right
  int throttle = rcToCmd255(ch2_throttle);   // -255 back ... +255 forward

  int leftCmd = 0, rightCmd = 0;

#if PURE_TANK_TURN
  // Rotate in place if steering commanded
  if (steer != 0) {
    leftCmd  =  steer;
    rightCmd = -steer;
  } else {
    leftCmd  = throttle;
    rightCmd = throttle;
  }
#else
  // Arcade mix (recommended): throttle + steer combine smoothly
  // Left  = throttle + steer
  // Right = throttle - steer
  leftCmd  = throttle + steer;
  rightCmd = throttle - steer;
#endif

  leftCmd  = constrain(leftCmd,  -PWM_MAX, PWM_MAX);
  rightCmd = constrain(rightCmd, -PWM_MAX, PWM_MAX);

  // Apply to motors: Left side = FL+BL, Right side = FR+BR
  setMotor(M_FL_DIR, M_FL_PWM, -leftCmd);
  setMotor(M_BL_DIR, M_BL_PWM, -leftCmd);

  setMotor(M_FR_DIR, M_FR_PWM, rightCmd);
  setMotor(M_BR_DIR, M_BR_PWM, rightCmd);
}

// -------------------------
// MAVLink Helpers
// -------------------------
void send_heartbeat() {
  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];

  mavlink_msg_heartbeat_pack(
    MY_SYSID, MY_COMPID, &msg,
    MAV_TYPE_GCS, MAV_AUTOPILOT_INVALID,
    MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 0, MAV_STATE_ACTIVE
  );

  const uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
  FC.write(buf, len);
}

void request_rcchannels_at_10hz() {
  const uint16_t MSG_ID = MAVLINK_MSG_ID_RC_CHANNELS;
  const float interval_us = 100000.0f; // 10Hz

  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];

  mavlink_msg_command_long_pack(
    MY_SYSID, MY_COMPID, &msg,
    target_sysid, target_compid,
    MAV_CMD_SET_MESSAGE_INTERVAL, 0,
    (float)MSG_ID, interval_us,
    0, 0, 0, 0, 0
  );

  const uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
  FC.write(buf, len);
}

void visualizeDirection(int rc1, int rc2) {
  String dir = "";
  if (rc2 > 1500) dir += "Forward ";
  else if (rc2 < 1500) dir += "Backward ";

  if (rc1 > 1500) dir += "Right";
  else if (rc1 < 1500) dir += "Left";

  if (dir == "") dir = "Neutral";

  String arrow = "";
  if (dir.indexOf("Forward")  != -1) arrow += " ↑";
  if (dir.indexOf("Backward") != -1) arrow += " ↓";
  if (dir.indexOf("Left")     != -1) arrow += " ←";
  if (dir.indexOf("Right")    != -1) arrow += " →";
  if (arrow == "") arrow = "•";

  Serial.printf("Direction: %-15s [%s]\n", dir.c_str(), arrow.c_str());
}

uint16_t speedToIntervalMs(int speed100) {
  speed100 = constrain(speed100, 0, 100);
  int ms = 100 - speed100;
  if (ms < 2) ms = 2;
  return (uint16_t)ms;
}

// -------------------------
// Setup / Loop
// -------------------------
void setup() {
  Serial.begin(USB_BAUD);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 2000) {}
  FC.begin(FC_BAUD);

  Serial.println(F("\n[Teensy] MAVLink RC + Morph + 4x DC Motor Tank Drive"));
  Serial.println(F("Requesting RC_CHANNELS @ 10 Hz after first FC heartbeat..."));

  // Servos
  myServoFL.attach(SERVO_PIN_FL);
  myServoFR.attach(SERVO_PIN_FR);
  myServoBR.attach(SERVO_PIN_BR);
  myServoBL.attach(SERVO_PIN_BL);

  morphPos = 0;
  morphTarget = 0;
  morphActive = false;
  applyMorphServos(morphPos);

  // Motors
  pinMode(M_FL_DIR, OUTPUT); pinMode(M_FL_PWM, OUTPUT);
  pinMode(M_FR_DIR, OUTPUT); pinMode(M_FR_PWM, OUTPUT);
  pinMode(M_BL_DIR, OUTPUT); pinMode(M_BL_PWM, OUTPUT);
  pinMode(M_BR_DIR, OUTPUT); pinMode(M_BR_PWM, OUTPUT);

  analogWriteResolution(8); // 0..255

  // Optional: increase PWM frequency for quieter motors (Teensy feature)
  // analogWriteFrequency(M_FL_PWM, 20000);
  // analogWriteFrequency(M_FR_PWM, 20000);
  // analogWriteFrequency(M_BL_PWM, 20000);
  // analogWriteFrequency(M_BR_PWM, 20000);

  stopAllMotors();
  rcTimeout = 0;

  Serial.println("Motors: FL(4,5) FR(7,8) BL(1,2) BR(10,11) [DIR,PWM]");
  Serial.println("PWM inverted: 0=max speed, 255=stop");
  Serial.println("Ready.");
}

void loop() {
  updateMorphMotion();

  // Failsafe
  if (rcTimeout > RC_TIMEOUT_MS) {
    stopAllMotors();
  }

  // Heartbeat
  if (hbTimer >= 1000) {
    hbTimer = 0;
    send_heartbeat();
  }

  while (FC.available() > 0) {
    static mavlink_message_t msg;
    static mavlink_status_t status;
    const uint8_t c = FC.read();

    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
      switch (msg.msgid) {

        case MAVLINK_MSG_ID_HEARTBEAT:
          if (!requested_rc_stream) {
            target_sysid  = msg.sysid;
            target_compid = msg.compid;
            request_rcchannels_at_10hz();
            requested_rc_stream = true;
            Serial.printf("Requested RC_CHANNELS from sys=%u comp=%u\n",
                          target_sysid, target_compid);
          }
          break;

        case MAVLINK_MSG_ID_RC_CHANNELS: {
          mavlink_rc_channels_t rc;
          mavlink_msg_rc_channels_decode(&msg, &rc);

          rcTimeout = 0; // got RC update

          // Arm/Disarm print
          if (rc.chan8_raw > 1500) Serial.println("Arm");
          else                     Serial.println("Disarm");

          // Morph on CH5
          int speed = 30;
          morphStepIntervalMs = speedToIntervalMs(speed);

          int newTarget = (rc.chan5_raw > 1500) ? 0 : 90;
          if (newTarget != morphTarget) {
            morphTarget = newTarget;
            morphActive = true;
            Serial.printf("Morph Target: %s | pos=%d -> %d\n",
                          (morphTarget == 0) ? "Down" : "Up",
                          morphPos, morphTarget);
          }
    
          // ---- Drive motors from RC ----
          driveFromRC(rc.chan1_raw, rc.chan2_raw);

          // Optional debug visualization
          visualizeDirection(rc.chan1_raw, rc.chan2_raw);
          Serial.println("----------------------");
        } break;

        default:
          break;
      }
    }
  }
}