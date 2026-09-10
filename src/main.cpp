#include <Arduino.h>
#include "display.h"
#include "relays.h"

Display display;
Relays relays;

const uint16_t RELAY_TIME = 500;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  display.init();
  display.print("battery", "charge");

  relays.init();
}

void turnRelay(uint8_t relay) {
  relays.turnOn(relay);
  delay(RELAY_TIME);
  relays.turnOff(relay);
  delay(RELAY_TIME);
}

void loop() {
  turnRelay(Relays::RELAY_1);
  turnRelay(Relays::RELAY_2);
  turnRelay(Relays::RELAY_3);
  turnRelay(Relays::RELAY_4);
}