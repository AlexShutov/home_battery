#include "battery_station.h"

// Инициализация: компоненты устройства из базового класса, затем привязка
// реле зарядок к машине (действия состояний управляют ими) и запуск
// стейт-машины зарядки (вход в начальное состояние — зарядка выключена).
void BatteryStation::init() {
  DeviceLogic::init();
  bindChargingRelays(relays);
  startDeviceStateMachine();
}

// Главный цикл: опрашивает состояние устройства; при изменении тарифного
// периода базовый класс вызывает updateState().
void BatteryStation::loop() {
  update();
}

// Считывает актуальное состояние: тарифный период — из выключателя, режим
// зарядки — из стейт-машины.
void BatteryStation::getState(BatteryStationState& out) {
  out.time_interval_type = time_switcher.getState();
  out.charging_mode = getChargingMode();
}

// Сообщает стейт-машине тарифный период из переданного состояния; режим
// зарядки напрямую не устанавливается — его определяет стейт-машина.
void BatteryStation::setState(const BatteryStationState& state) {
  setTariff(state.time_interval_type);
}

// Реакция на изменение состояния устройства: при смене тарифного периода
// стейт-машина переводит зарядку в соответствующее состояние.
void BatteryStation::updateState() {
  setTariff(device_state.time_interval_type);
}
