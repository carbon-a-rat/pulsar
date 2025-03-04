#include <Arduino.h>
#include <vutils.h>
void setup() {
  pinMode(0, OUTPUT);
}

void loop() {
  digitalWrite(0, HIGH);
  delay(BLINK_TIMING);
  digitalWrite(0, LOW);
  delay(BLINK_TIMING);
}
