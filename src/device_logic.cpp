#include "device_logic.h"

const uint16_t DeviceLogic::RELAY_TIME;
const uint16_t DeviceLogic::STATE_CHANGE_DELAY;

void DeviceLogic::init() {
  pinMode(LED_BUILTIN, OUTPUT);

  screen.init(display);
  relays.init();
  time_switcher.init();
  readDeviceState(device_state);
  new_device_state = device_state;
  screen.print_state(device_state);
}

// Считывает актуальное состояние из TimeSwitcher и Relays.
void DeviceLogic::readDeviceState(DeviceState& state) {
  state.time_interval_type = time_switcher.getState();
  relays.getState(state.relays);
}

// Считывает актуальное состояние из TimeSwitcher и Relays; при изменении тарифного периода вызывает updateState().
void DeviceLogic::update() {
  readDeviceState(new_device_state);

  // Сравнивание только через оператор == структуры состояния.
  if (!(new_device_state.time_interval_type == device_state.time_interval_type)) {
    device_state = new_device_state;
    screen.print_state(device_state);
    updateState();
    delay(STATE_CHANGE_DELAY);
  }
  device_state = new_device_state;
}
