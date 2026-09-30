#include "battery_reader.h"

BatteryReader::BatteryReader(BmsReader& bms) : bms(bms) {
  state.current = 0.0f;
  state.voltage = 0.0f;
  state.isTemperatureOk = true;
  state.chargePercentage = 0.0f;
}

// Проверяет блок тревог БМС: температура батареи в норме, когда нет ни
// одной тревоги заряда/разряда по высокой/низкой температуре и исправен
// модуль температурного датчика.
static bool isBatteryTemperatureOk(const BmsReader::Alarms& alarms) {
  return !alarms.levelOneChargeTempTooHigh && !alarms.levelTwoChargeTempTooHigh &&
         !alarms.levelOneChargeTempTooLow && !alarms.levelTwoChargeTempTooLow &&
         !alarms.levelOneDischargeTempTooHigh &&
         !alarms.levelTwoDischargeTempTooHigh &&
         !alarms.levelOneDischargeTempTooLow &&
         !alarms.levelTwoDischargeTempTooLow &&
         !alarms.failureOfTemperatureSensorModule;
}

bool BatteryReader::update() {
  if (!bms.update()) {
    // БМС не отвечает (батарея разряжена/отключилась): показания не
    // обновляются, остаётся состояние прошлого успешного чтения.
    return false;
  }

  state.current = bms.getTelemetry().packCurrent;
  state.voltage = bms.getTelemetry().packVoltage;
  state.isTemperatureOk = isBatteryTemperatureOk(bms.getAlarms());
  state.chargePercentage = percentage_calculator.calculate(state.voltage);
  return true;
}

void BatteryReader::getState(BatteryState& out) {
  out = state;
}
