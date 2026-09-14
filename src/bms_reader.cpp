#include "bms_reader.h"

// Привязываем драйвер к единственному аппаратному UART контроллера (Serial).
BmsReader::BmsReader() : bms(Serial), connectionError(false) {}

void BmsReader::init() {
  // Настраивает последовательный порт (9600, 8N1) по спецификации Daly.
  bms.Init();
}

bool BmsReader::readBmsValues(BmsReadings& readings) {
  // Запрашивает у БМС все блоки данных; update() заполняет структуры get/alarm.
  bool ok = bms.update();
  // БМС не ответила по UART (считаем, что батарея разряжена и БМС отключилась) —
  // запоминаем ошибку; успешное чтение её сбрасывает.
  connectionError = !ok;
  // Переносим актуальные значения в выходной параметр независимо от результата
  // (частично обновлённые данные лучше пустых при обрыве связи).
  copyFromLibrary(readings);
  return ok;
}

bool BmsReader::isConnectionError() const {
  return connectionError;
}

void BmsReader::copyFromLibrary(BmsReadings& readings) const {
  // Команда 0x90: напряжение, ток, SOC.
  readings.packVoltage = bms.get.packVoltage;
  readings.packCurrent = bms.get.packCurrent;
  readings.packSOC = bms.get.packSOC;

  // Команда 0x91: минимальное/максимальное напряжение элементов и их номера.
  readings.maxCellmV = bms.get.maxCellmV;
  readings.maxCellVNum = bms.get.maxCellVNum;
  readings.minCellmV = bms.get.minCellmV;
  readings.minCellVNum = bms.get.minCellVNum;
  readings.cellDiff = bms.get.cellDiff;

  // Команда 0x92: температура.
  readings.tempMax = bms.get.tempMax;
  readings.tempMin = bms.get.tempMin;
  readings.tempAverage = bms.get.tempAverage;

  // Команда 0x93: статус заряда/разряда, MOS-ключи, пульс, остаточная ёмкость.
  readings.chargeDischargeStatus = bms.get.chargeDischargeStatus;
  readings.chargeFetState = bms.get.chargeFetState;
  readings.disChargeFetState = bms.get.disChargeFetState;
  readings.bmsHeartBeat = bms.get.bmsHeartBeat;
  readings.resCapacitymAh = bms.get.resCapacitymAh;

  // Команда 0x94: служебная информация.
  readings.numberOfCells = bms.get.numberOfCells;
  readings.numOfTempSensors = bms.get.numOfTempSensors;
  readings.chargeState = bms.get.chargeState;
  readings.loadState = bms.get.loadState;
  for (uint8_t i = 0; i < 8u; ++i) {
    readings.dIO[i] = bms.get.dIO[i];
  }
  readings.bmsCycles = bms.get.bmsCycles;

  // Команда 0x95: напряжения всех элементов.
  for (uint8_t i = 0; i < MAX_NUMBER_CELLS; ++i) {
    readings.cellVmV[i] = bms.get.cellVmV[i];
  }

  // Команда 0x96: температуры всех датчиков.
  for (uint8_t i = 0; i < MAX_NUMBER_TEMP_SENSORS; ++i) {
    readings.cellTemperature[i] = bms.get.cellTemperature[i];
  }

  // Команда 0x97: состояние балансировки.
  for (uint8_t i = 0; i < MAX_NUMBER_CELLS; ++i) {
    readings.cellBalanceState[i] = bms.get.cellBalanceState[i];
  }
  readings.cellBalanceActive = bms.get.cellBalanceActive;

  // Команда 0x98: все тревожные флаги.
  // 0x00
  readings.levelOneCellVoltageTooHigh = bms.alarm.levelOneCellVoltageTooHigh;
  readings.levelTwoCellVoltageTooHigh = bms.alarm.levelTwoCellVoltageTooHigh;
  readings.levelOneCellVoltageTooLow = bms.alarm.levelOneCellVoltageTooLow;
  readings.levelTwoCellVoltageTooLow = bms.alarm.levelTwoCellVoltageTooLow;
  readings.levelOnePackVoltageTooHigh = bms.alarm.levelOnePackVoltageTooHigh;
  readings.levelTwoPackVoltageTooHigh = bms.alarm.levelTwoPackVoltageTooHigh;
  readings.levelOnePackVoltageTooLow = bms.alarm.levelOnePackVoltageTooLow;
  readings.levelTwoPackVoltageTooLow = bms.alarm.levelTwoPackVoltageTooLow;
  // 0x01
  readings.levelOneChargeTempTooHigh = bms.alarm.levelOneChargeTempTooHigh;
  readings.levelTwoChargeTempTooHigh = bms.alarm.levelTwoChargeTempTooHigh;
  readings.levelOneChargeTempTooLow = bms.alarm.levelOneChargeTempTooLow;
  readings.levelTwoChargeTempTooLow = bms.alarm.levelTwoChargeTempTooLow;
  readings.levelOneDischargeTempTooHigh = bms.alarm.levelOneDischargeTempTooHigh;
  readings.levelTwoDischargeTempTooHigh = bms.alarm.levelTwoDischargeTempTooHigh;
  readings.levelOneDischargeTempTooLow = bms.alarm.levelOneDischargeTempTooLow;
  readings.levelTwoDischargeTempTooLow = bms.alarm.levelTwoDischargeTempTooLow;
  // 0x02
  readings.levelOneChargeCurrentTooHigh = bms.alarm.levelOneChargeCurrentTooHigh;
  readings.levelTwoChargeCurrentTooHigh = bms.alarm.levelTwoChargeCurrentTooHigh;
  readings.levelOneDischargeCurrentTooHigh = bms.alarm.levelOneDischargeCurrentTooHigh;
  readings.levelTwoDischargeCurrentTooHigh = bms.alarm.levelTwoDischargeCurrentTooHigh;
  readings.levelOneStateOfChargeTooHigh = bms.alarm.levelOneStateOfChargeTooHigh;
  readings.levelTwoStateOfChargeTooHigh = bms.alarm.levelTwoStateOfChargeTooHigh;
  readings.levelOneStateOfChargeTooLow = bms.alarm.levelOneStateOfChargeTooLow;
  readings.levelTwoStateOfChargeTooLow = bms.alarm.levelTwoStateOfChargeTooLow;
  // 0x03
  readings.levelOneCellVoltageDifferenceTooHigh = bms.alarm.levelOneCellVoltageDifferenceTooHigh;
  readings.levelTwoCellVoltageDifferenceTooHigh = bms.alarm.levelTwoCellVoltageDifferenceTooHigh;
  readings.levelOneTempSensorDifferenceTooHigh = bms.alarm.levelOneTempSensorDifferenceTooHigh;
  readings.levelTwoTempSensorDifferenceTooHigh = bms.alarm.levelTwoTempSensorDifferenceTooHigh;
  // 0x04
  readings.chargeFETTemperatureTooHigh = bms.alarm.chargeFETTemperatureTooHigh;
  readings.dischargeFETTemperatureTooHigh = bms.alarm.dischargeFETTemperatureTooHigh;
  readings.failureOfChargeFETTemperatureSensor = bms.alarm.failureOfChargeFETTemperatureSensor;
  readings.failureOfDischargeFETTemperatureSensor = bms.alarm.failureOfDischargeFETTemperatureSensor;
  readings.failureOfChargeFETAdhesion = bms.alarm.failureOfChargeFETAdhesion;
  readings.failureOfDischargeFETAdhesion = bms.alarm.failureOfDischargeFETAdhesion;
  // В библиотеке опечатка: имя поля с двойной "T".
  readings.failureOfChargeFETBreaker = bms.alarm.failureOfChargeFETTBreaker;
  readings.failureOfDischargeFETBreaker = bms.alarm.failureOfDischargeFETBreaker;
  // 0x05
  readings.failureOfAFEAcquisitionModule = bms.alarm.failureOfAFEAcquisitionModule;
  readings.failureOfVoltageSensorModule = bms.alarm.failureOfVoltageSensorModule;
  readings.failureOfTemperatureSensorModule = bms.alarm.failureOfTemperatureSensorModule;
  readings.failureOfEEPROMStorageModule = bms.alarm.failureOfEEPROMStorageModule;
  readings.failureOfRealtimeClockModule = bms.alarm.failureOfRealtimeClockModule;
  readings.failureOfPrechargeModule = bms.alarm.failureOfPrechargeModule;
  readings.failureOfVehicleCommunicationModule = bms.alarm.failureOfVehicleCommunicationModule;
  readings.failureOfIntranetCommunicationModule = bms.alarm.failureOfIntranetCommunicationModule;
  // 0x06
  readings.failureOfCurrentSensorModule = bms.alarm.failureOfCurrentSensorModule;
  readings.failureOfMainVoltageSensorModule = bms.alarm.failureOfMainVoltageSensorModule;
  readings.failureOfShortCircuitProtection = bms.alarm.failureOfShortCircuitProtection;
  readings.failureOfLowVoltageNoCharging = bms.alarm.failureOfLowVoltageNoCharging;
}
