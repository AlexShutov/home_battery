#include <Arduino.h>
#include "display.h"

Display display;

int secondsLeft = 100;
unsigned long lastUpdate = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  display.init();
  display.print("hello", String(secondsLeft) + " seconds");
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate >= 1000) {
    lastUpdate = now;

    if (secondsLeft > 0) {
      secondsLeft--;
      display.print("hello", String(secondsLeft) + " seconds");
    }
  }
}
