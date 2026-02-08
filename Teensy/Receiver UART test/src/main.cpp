#include <Arduino.h>

void setup() {
  Serial.begin(115200);      // USB debug
  Serial5.begin(420000);     // CRSF
}

void loop() {
  if (Serial5.available()) {
    uint8_t b = Serial5.read();
    Serial.print(b, HEX);
    Serial.print(" ");
  }
}
