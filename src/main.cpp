#include <Arduino.h>
#include "display.h"

Display display;

unsigned long lastUpdate = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  display.init();
  display.print("battery", "charge");
}

void loop() {
  // unsigned long now = millis();

  // if (now - lastUpdate >= 1000) {
  //   lastUpdate = now;
  //   display.print("battery", "charge");
  // }
}