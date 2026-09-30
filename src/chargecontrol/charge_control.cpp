#include "charge_control.h"

#include "charge_target_calculator.h"
#include "relay_state.h"

// Буфер состояния реле для применения флагов зарядок: локальные объекты
// неинтегральных типов создавать нельзя (правило «Память»).
static RelayState relay_target;

// Инициализирует реле зарядок; начальное состояние — все выключены.
void ChargeControl::init() {
  relays.init();
  state.isChargeOn = false;
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    state.activeRelays[i] = 0;
  }
}

// Рассчитывает целевое состояние зарядок (обёртка над свободной функцией;
// глобальный квалификатор отличает её от одноимённого метода).
void ChargeControl::calculateTargetState(const BatteryState& battery,
                                         ChargeControlState& out,
                                         bool isMinimalCurrent) {
  ::calculateTargetState(battery, out, isMinimalCurrent);
}

// Применяет целевое состояние к зарядкам: при перегреве — все выключены,
// иначе включаются зарядки с флагом 1.
void ChargeControl::activateTargetState(const BatteryState& battery,
                                        const ChargeControlState& target) {
  if (!battery.isTemperatureOk) {
    state.isChargeOn = false;
    for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
      state.activeRelays[i] = 0;
      relay_target.relays[i] = false;
    }
    relays.setState(relay_target);
    return;
  }
  setState(target);
}

// Считывает последнее применённое состояние зарядок.
void ChargeControl::getState(ChargeControlState& out) const {
  out = state;
}

// Устанавливает состояние зарядок: обновляет внутреннее состояние и реле.
void ChargeControl::setState(const ChargeControlState& newState) {
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    relay_target.relays[i] = (newState.activeRelays[i] != 0);
    state.activeRelays[i] = (newState.activeRelays[i] != 0) ? 1 : 0;
  }
  state.isChargeOn = newState.isChargeOn;
  relays.setState(relay_target);
}

// Считывает фактическое состояние реле зарядок.
void ChargeControl::getRelayState(RelayState& out) const {
  relays.getState(out);
}
