#include "battery_station.h"

#include "chargecontrol/charge_control.h"
#include "device/battery/battery_reader.h"
#include "statemachine/device_state_machine.h"

// Инициализация: компоненты устройства из базового класса, затем привязка
// контроля зарядок к машине (действия состояний рассчитывают и применяют
// целевое состояние зарядок) и запуск стейт-машины зарядки (вход в
// начальное состояние — зарядка выключена).
void BatteryStation::init() {
  DeviceLogic::init();
  bindChargeControl(charge_control);
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
  updateBatterySnapshot();
  setTariff(state.time_interval_type);
}

// Реакция на изменение состояния устройства: сначала обновляет показания
// батареи и кладёт свежий снимок в стейт-машину (по нему состояния
// рассчитывают целевое состояние зарядок), затем сообщает машине новый
// тарифный период.
void BatteryStation::updateState() {
  updateBatterySnapshot();
  setTariff(device_state.time_interval_type);
}

// Опрашивает БМС и обновляет снимок показаний батареи в стейт-машине;
// при ошибке связи BatteryReader отдаёт показания последнего успешного
// чтения.
void BatteryStation::updateBatterySnapshot() {
  battery_reader.update();
  battery_reader.getState(battery_state);
  setBatterySnapshot(battery_state);
}
