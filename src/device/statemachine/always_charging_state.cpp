#include "always_charging_state.h"

// Целевые состояния реле режима постоянной зарядки. Хранятся статически —
// локальные объекты неинтегральных типов создавать нельзя (правило «Память»).
static RelayState relays_on;
static RelayState relays_off;

// Действие входа: постоянная зарядка — фиксирует режим и включает все реле
// зарядок (setState обновляет и внутреннее состояние, и ножки).
void AlwaysChargingState::entry() {
  setActiveChargingMode(CHARGING_MODE_ALWAYS);
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    relays_on.relays[i] = true;
  }
  applyChargingRelayState(relays_on);
}

// Действие выхода: постоянная зарядка закончилась — выключает все реле.
// Выполняется до входа в следующее состояние, которое применит собственную
// конфигурацию реле.
void AlwaysChargingState::exit() {
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    relays_off.relays[i] = false;
  }
  applyChargingRelayState(relays_off);
}
