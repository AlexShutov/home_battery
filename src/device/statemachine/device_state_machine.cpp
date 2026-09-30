#include "device_state_machine.h"

#include "always_charging_state.h"
#include "chargecontrol/charge_control.h"
#include "charging_off_state.h"
#include "max_power_charging_state.h"
#include "minimal_power_charging_state.h"
#include "overheating_state.h"

// Начальное состояние — зарядка выключена.
FSM_INITIAL_STATE(DeviceStateMachine, ChargingOffState)

// Режим зарядки текущего состояния машины. Обновляется только действиями
// входа состояний.
static ChargingMode active_mode = CHARGING_MODE_OFF;

// Событие смены тарифа существует в единственном экземпляре в статической
// памяти: локальные объекты неинтегральных типов создавать нельзя.
static TariffEvent tariff_event;

// Событие обновления показаний батареи: тоже единственный экземпляр.
static BatterySnapshotEvent battery_event;

// Привязанный контроль зарядок: nullptr, пока машина не связана с
// устройством; действия состояний при этом зарядки не меняют.
static ChargeControl* charge_control = nullptr;

// Снимок показаний батареи для расчётов состояний: единственный экземпляр
// в статической памяти.
static BatteryState battery_snapshot;

// Таблица переходов машины: тарифный период -> состояние зарядки. Если машина
// уже в целевом состоянии, переход не выполняется, чтобы не перезапускать
// действие входа. Состояние перегрева из таблицы недостижимо: в него переводит
// событие от датчика температуры (будет добавлено вместе с температурной
// логикой), а не смена тарифа.
void DeviceStateMachine::react(TariffEvent const& event) {
  switch (event.tariff) {
    case CHEAP:
      if (!is_in_state<MaxPowerChargingState>()) {
        transit<MaxPowerChargingState>();
      }
      break;
    case MIDDLE:
      if (!is_in_state<MinimalPowerChargingState>()) {
        transit<MinimalPowerChargingState>();
      }
      break;
    case EXPENSIVE:
      if (!is_in_state<ChargingOffState>()) {
        transit<ChargingOffState>();
      }
      break;
    case FORCE_CHARGING:
      if (!is_in_state<AlwaysChargingState>()) {
        transit<AlwaysChargingState>();
      }
      break;
  }
}

// Привязывает контроль зарядок к машине.
void bindChargeControl(ChargeControl& control) {
  charge_control = &control;
}

// Обновляет снимок показаний батареи.
void setBatterySnapshot(const BatteryState& battery) {
  battery_snapshot = battery;
}

// Возвращает привязанный контроль зарядок.
ChargeControl* getChargeControl() {
  return charge_control;
}

// Возвращает снимок показаний батареи.
const BatteryState& getBatterySnapshot() {
  return battery_snapshot;
}

// Запускает стейт-машину устройства; вызов статического метода библиотеки
// обёрнут в свободную функцию, как того требуют правила проекта.
void startDeviceStateMachine() {
  DeviceStateMachine::start();
}

// Отправляет машине событие смены тарифного периода.
void setTariff(TimeInterval tariff) {
  tariff_event.tariff = tariff;
  DeviceStateMachine::dispatch(tariff_event);
}

// Отправляет машине событие обновления показаний батареи.
void notifyBatterySnapshot() {
  DeviceStateMachine::dispatch(battery_event);
}

// Возвращает режим зарядки текущего состояния машины.
ChargingMode getChargingMode() {
  return active_mode;
}

// Обновляет текущий режим зарядки.
void setActiveChargingMode(ChargingMode mode) {
  active_mode = mode;
}
