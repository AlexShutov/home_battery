#include "minimal_power_charging_state.h"

#include "chargecontrol/charge_control.h"
#include "chargecontrol/charge_control_state.h"
#include "chargecontrol/relay_state.h"
#include "device_state_machine.h"

// Порог достаточного заряда, %: выше — пользователю хватит, зарядки
// выключаются.
static const float HIGH_CHARGE_PERCENT = 60.0f;

// Порог низкого заряда, %: ниже — высокое потребление, батарея дозаряжается
// всеми зарядками до выхода из состояния.
static const float LOW_CHARGE_PERCENT = 50.0f;

// Целевое состояние зарядок. Хранится статически — локальные объекты
// неинтегральных типов создавать нельзя (правило «Память»).
static ChargeControlState minimal_target_state;

MinimalPowerChargingState::MinimalPowerChargingState() : isHighConsumption(false) {}

// Действие входа: фиксирует режим зарядки и сразу оценивает уровень заряда.
void MinimalPowerChargingState::entry() {
  setActiveChargingMode(CHARGING_MODE_MINIMAL);
  evaluate();
}

// Действие выхода: флаг высокого потребления живёт только внутри состояния.
void MinimalPowerChargingState::exit() {
  isHighConsumption = false;
}

// Показания батареи обновились — переоцениваем конфигурацию зарядок.
void MinimalPowerChargingState::react(BatterySnapshotEvent const&) {
  evaluate();
}

// Переоценка конфигурации зарядок по текущему уровню заряда.
void MinimalPowerChargingState::evaluate() {
  ChargeControl* control = getChargeControl();
  if (control == nullptr) {
    return;
  }
  const BatteryState& battery = getBatterySnapshot();

  // Уже дозаряжаем: электричество не очень дорогое, продолжаем до выхода
  // из состояния, уровень заряда повторно не проверяется.
  if (isHighConsumption) {
    control->calculateTargetState(battery, minimal_target_state, false);
    control->activateTargetState(battery, minimal_target_state);
    return;
  }

  if (battery.chargePercentage > HIGH_CHARGE_PERCENT) {
    // Пользователь тратит электричество не слишком активно — заряда
    // хватит, все зарядки выключаются.
    minimal_target_state.isChargeOn = false;
    for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
      minimal_target_state.activeRelays[i] = 0;
    }
    control->activateTargetState(battery, minimal_target_state);
    return;
  }

  if (battery.chargePercentage < LOW_CHARGE_PERCENT) {
    // Высокое потребление: поднимаем флаг и дозаряжаем батарею полностью
    // всеми зарядками.
    isHighConsumption = true;
    control->calculateTargetState(battery, minimal_target_state, false);
    control->activateTargetState(battery, minimal_target_state);
    return;
  }

  // Зона гистерезиса 50–60%: конфигурацию зарядок не меняем.
}
