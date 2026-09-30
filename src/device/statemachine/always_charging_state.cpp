#include "always_charging_state.h"

#include "chargecontrol/charge_control.h"
#include "chargecontrol/charge_control_state.h"
#include "device_state_machine.h"

// Целевое состояние зарядок для входа и выхода. Хранится статически —
// локальные объекты неинтегральных типов создавать нельзя (правило «Память»).
static ChargeControlState target_state;

// Действие входа: постоянная зарядка — фиксирует режим и включает все
// зарядки. Целевое состояние рассчитывается с isMinimalCurrent = false:
// при нормальной температуре включаются все зарядки независимо от тока
// (в том числе когда заряд уже идёт от внешней зарядки, которой нет в
// списке реле); при перегреве activateTargetState выключает всё.
void AlwaysChargingState::entry() {
  setActiveChargingMode(CHARGING_MODE_ALWAYS);
  ChargeControl* control = getChargeControl();
  if (control == nullptr) {
    return;
  }
  control->calculateTargetState(getBatterySnapshot(), target_state, false);
  control->activateTargetState(getBatterySnapshot(), target_state);
}

// Действие выхода: постоянная зарядка закончилась — выключает все зарядки.
// Переопределяются сами флаги зарядок, температура повторно не проверяется:
// следующее состояние при входе рассчитает и применит свою конфигурацию.
void AlwaysChargingState::exit() {
  ChargeControl* control = getChargeControl();
  if (control == nullptr) {
    return;
  }
  target_state.isChargeOn = false;
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    target_state.activeRelays[i] = 0;
  }
  control->setState(target_state);
}
