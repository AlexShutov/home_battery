#include "device_logic.h"

const uint16_t DeviceLogic::RELAY_TIME;
const uint16_t DeviceLogic::STATE_CHANGE_DELAY;

void DeviceLogic::init() {
  pinMode(LED_BUILTIN, OUTPUT);

  screen.init(display);
  relays.init();
  time_switcher.init();
  bms_reader.init();
  readDeviceState(device_state);
  new_device_state = device_state;
  screen.print_state(device_state);
}

// Считывает актуальное состояние из TimeSwitcher и Relays.
void DeviceLogic::readDeviceState(DeviceState& state) {
  state.time_interval_type = time_switcher.getState();
  relays.getState(state.relays);
}

// Считывает актуальное состояние из TimeSwitcher и Relays; при изменении состояния вызывает updateState().
void DeviceLogic::update() {
  readDeviceState(new_device_state);

  // Сравнивание только через оператор == структуры состояния.
  if (!(new_device_state == device_state)) {
    device_state = new_device_state;
    screen.print_state(device_state);
    updateState();
    // После реакции перечитываем фактическое состояние (updateState мог изменить реле),
    // чтобы device_state не оставался устаревшим.
    readDeviceState(device_state);
    delay(STATE_CHANGE_DELAY);
  }
}
