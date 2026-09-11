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
DeviceState new_device_state;

const uint16_t RELAY_TIME = 500;

// Задержка после изменения состояния устройства, мс.
const uint16_t STATE_CHANGE_DELAY = 1000;

void readDeviceState(DeviceState& state);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  screen.init(display);
  relays.init();
  time_switcher.init();
  readDeviceState(device_state);
  new_device_state = device_state;
}

void turnRelay(uint8_t relay) {
  relays.turnOn(relay);
  relays.getState(new_device_state.relays);
  screen.print_state(new_device_state);
  delay(RELAY_TIME);
  relays.turnOff(relay);
  relays.getState(new_device_state.relays);
  screen.print_state(new_device_state);
  delay(RELAY_TIME);
}

// Действия при изменении состояния устройства: цикл всех реле.
void updateState() {
  turnRelay(Relays::RELAY_1);
  turnRelay(Relays::RELAY_2);
  turnRelay(Relays::RELAY_3);
  turnRelay(Relays::RELAY_4);
}

void readDeviceState(DeviceState& state) {
  state.time_interval_type = time_switcher.getState();
  relays.getState(state.relays);
}

// Считывает актуальное состояние из TimeSwitcher и Relays; при изменении вызывает updateState().
void update() {
  readDeviceState(new_device_state);

  // Сравнивание только через оператор == структуры состояния.
  if (!(new_device_state.time_interval_type == device_state.time_interval_type)) {
    device_state = new_device_state;
    screen.print_state(device_state);
    updateState();
    delay(STATE_CHANGE_DELAY);
  }
}

void loop() {
  update();
  delay(STATE_CHANGE_DELAY);
}