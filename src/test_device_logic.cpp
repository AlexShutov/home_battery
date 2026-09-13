#include "test_device_logic.h"

void TestDeviceLogic::init() {
  // Инициализация компонентов выполняется базовым классом DeviceLogic.
  DeviceLogic::init();
}

// Периодически опрашивает состояние устройства с задержкой STATE_CHANGE_DELAY между циклами.
void TestDeviceLogic::loop() {
  update();
  delay(STATE_CHANGE_DELAY);
}

// Кратковременно включает и выключает все реле по очереди.
void TestDeviceLogic::updateState() {
  turnRelay(Relays::RELAY_1);
  turnRelay(Relays::RELAY_2);
  turnRelay(Relays::RELAY_3);
  turnRelay(Relays::RELAY_4);
}

void TestDeviceLogic::turnRelay(uint8_t relay) {
  relays.turnOn(relay);
  relays.getState(new_device_state.relays);
  screen.print_state(new_device_state);
  delay(RELAY_TIME);
  relays.turnOff(relay);
  relays.getState(new_device_state.relays);
  screen.print_state(new_device_state);
  delay(RELAY_TIME);
}
