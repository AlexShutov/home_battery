#include <Arduino.h>
#include "display.h"
#include "device_screen.h"
#include "relays.h"
#include "time_switcher.h"

Display display;
Relays relays;
DeviceScreen screen;
TimeSwitcher time_switcher;
DeviceState device_state;

const uint16_t RELAY_TIME = 500;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  screen.init(display);
  relays.init();
  time_switcher.init();
}

void turnRelay(uint8_t relay) {
  relays.turnOn(relay);
  relays.getState(device_state.relays);
  screen.print_state(device_state);
  delay(RELAY_TIME);
  relays.turnOff(relay);
  relays.getState(device_state.relays);
  screen.print_state(device_state);
  delay(RELAY_TIME);
}

void loop() {
  turnRelay(Relays::RELAY_1);
  turnRelay(Relays::RELAY_2);
  turnRelay(Relays::RELAY_3);
  turnRelay(Relays::RELAY_4);
}