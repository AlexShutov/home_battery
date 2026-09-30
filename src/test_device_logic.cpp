#include "test_device_logic.h"

#include "chargecontrol/charge_control_state.h"
#include "chargecontrol/relay_state.h"

// Состояние зарядок для теста реле: включаем по одной. Хранится статически —
// локальные объекты неинтегральных типов создавать нельзя (правило «Память»).
static ChargeControlState test_state;

// Заполняет флаги зарядок: включена только указанная.
static void setSingleRelay(uint8_t relay) {
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    test_state.activeRelays[i] = (i == relay) ? 1 : 0;
  }
}

void TestDeviceLogic::init() {
  // Инициализация компонентов выполняется базовым классом DeviceLogic.
  DeviceLogic::init();
}

// Периодически опрашивает состояние устройства с задержкой STATE_CHANGE_DELAY между циклами.
void TestDeviceLogic::loop() {
  update();
  delay(STATE_CHANGE_DELAY);
}

// Кратковременно включает и выключает все зарядки по очереди.
void TestDeviceLogic::updateState() {
  turnRelay(Relays::RELAY_1);
  turnRelay(Relays::RELAY_2);
  turnRelay(Relays::RELAY_3);
  turnRelay(Relays::RELAY_4);
}

void TestDeviceLogic::turnRelay(uint8_t relay) {
  test_state.isChargeOn = true;
  setSingleRelay(relay);
  charge_control.setState(test_state);
  charge_control.getRelayState(new_device_state.relays);
  screen.print_state(new_device_state);
  delay(RELAY_TIME);

  test_state.isChargeOn = false;
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    test_state.activeRelays[i] = 0;
  }
  charge_control.setState(test_state);
  charge_control.getRelayState(new_device_state.relays);
  screen.print_state(new_device_state);
  delay(RELAY_TIME);
}
