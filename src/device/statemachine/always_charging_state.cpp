#include "always_charging_state.h"

#include "chargecontrol/charge_control.h"
#include "chargecontrol/charge_control_state.h"
#include "device_state_machine.h"

// Целевое состояние зарядок для входа. Хранится статически — локальные
// объекты неинтегральных типов создавать нельзя (правило «Память»).
static ChargeControlState always_target_state;

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
  control->calculateTargetState(getBatterySnapshot(), always_target_state, false);
  control->activateTargetState(getBatterySnapshot(), always_target_state);
}

// Действия выхода нет: смена тарифного периода выполняется таблицей
// переходов (transit в целевое состояние), и exit здесь не должен трогать
// зарядки — иначе при переходе, например, в максимальную зарядку реле
// бессмысленно щёлкнули бы «выключить — тут же включить». Целевую
// конфигурацию применяет entry() следующего состояния.
