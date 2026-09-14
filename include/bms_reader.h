#pragma once

#include <Arduino.h>
#include <daly-bms-uart.h>

// Полное значение телеметрии Daly BMS: все данные, которые БМС передаёт по UART
// (зеркалирует структуры get и alarm библиотеки daly-bms-uart). Использует только
// интегральные/float-типы; массивы размеруются по максимуму библиотеки.
struct BmsReadings {
    // --- данные команды 0x90 ---
    // Напряжение сборки, В.
    float packVoltage;
    // Ток: положительный — заряд, отрицательный — разряд, А.
    float packCurrent;
    // Уровень заряда, % (SOC).
    float packSOC;

    // --- данные команды 0x91 ---
    // Напряжение самого «высокого» элемента, мВ.
    float maxCellmV;
    // Номер элемента с максимальным напряжением (с 1).
    int maxCellVNum;
    // Напряжение самого «низкого» элемента, мВ.
    float minCellmV;
    // Номер элемента с минимальным напряжением (с 1).
    int minCellVNum;
    // Разброс напряжений элементов, мВ.
    float cellDiff;

    // --- данные команды 0x92 ---
    // Максимальная температура, °C.
    int tempMax;
    // Минимальная температура, °C.
    int tempMin;
    // Средняя температура, °C.
    float tempAverage;

    // --- данные команды 0x93 ---
    // Статус заряда/разряда: 0 — stationary, 1 — charge, 2 — discharge.
    uint8_t chargeDischargeStatus;
    // Состояние MOS-ключа заряда: true — открыт.
    bool chargeFetState;
    // Состояние MOS-ключа разряда: true — открыт.
    bool disChargeFetState;
    // Пульс БМС (0..255).
    int bmsHeartBeat;
    // Остаточная ёмкость, мА·ч.
    int resCapacitymAh;

    // --- данные команды 0x94 ---
    // Количество элементов в сборке.
    int numberOfCells;
    // Количество датчиков температуры.
    int numOfTempSensors;
    // Подключён ли зарядное устройство.
    bool chargeState;
    // Подключена ли нагрузка.
    bool loadState;
    // Состояния цифровых выходов БМС (8 бит).
    bool dIO[8];
    // Количество циклов заряда/разряда.
    int bmsCycles;

    // --- данные команды 0x95 ---
    // Напряжения всех элементов, мВ.
    float cellVmV[MAX_NUMBER_CELLS];

    // --- данные команды 0x96 ---
    // Температуры всех датчиков, °C.
    int cellTemperature[MAX_NUMBER_TEMP_SENSORS];

    // --- данные команды 0x97 ---
    // Состояние балансировки каждого элемента.
    bool cellBalanceState[MAX_NUMBER_CELLS];
    // Активна ли балансировка хотя бы по одному элементу.
    bool cellBalanceActive;

    // --- тревоги, данные команды 0x98 ---
    // 0x00: напряжения
    bool levelOneCellVoltageTooHigh;
    bool levelTwoCellVoltageTooHigh;
    bool levelOneCellVoltageTooLow;
    bool levelTwoCellVoltageTooLow;
    bool levelOnePackVoltageTooHigh;
    bool levelTwoPackVoltageTooHigh;
    bool levelOnePackVoltageTooLow;
    bool levelTwoPackVoltageTooLow;
    // 0x01: температуры заряда/разряда
    bool levelOneChargeTempTooHigh;
    bool levelTwoChargeTempTooHigh;
    bool levelOneChargeTempTooLow;
    bool levelTwoChargeTempTooLow;
    bool levelOneDischargeTempTooHigh;
    bool levelTwoDischargeTempTooHigh;
    bool levelOneDischargeTempTooLow;
    bool levelTwoDischargeTempTooLow;
    // 0x02: токи и SOC
    bool levelOneChargeCurrentTooHigh;
    bool levelTwoChargeCurrentTooHigh;
    bool levelOneDischargeCurrentTooHigh;
    bool levelTwoDischargeCurrentTooHigh;
    bool levelOneStateOfChargeTooHigh;
    bool levelTwoStateOfChargeTooHigh;
    bool levelOneStateOfChargeTooLow;
    bool levelTwoStateOfChargeTooLow;
    // 0x03: разброс
    bool levelOneCellVoltageDifferenceTooHigh;
    bool levelTwoCellVoltageDifferenceTooHigh;
    bool levelOneTempSensorDifferenceTooHigh;
    bool levelTwoTempSensorDifferenceTooHigh;
    // 0x04: MOS-ключи
    bool chargeFETTemperatureTooHigh;
    bool dischargeFETTemperatureTooHigh;
    bool failureOfChargeFETTemperatureSensor;
    bool failureOfDischargeFETTemperatureSensor;
    bool failureOfChargeFETAdhesion;
    bool failureOfDischargeFETAdhesion;
    bool failureOfChargeFETBreaker;
    bool failureOfDischargeFETBreaker;
    // 0x05: модули
    bool failureOfAFEAcquisitionModule;
    bool failureOfVoltageSensorModule;
    bool failureOfTemperatureSensorModule;
    bool failureOfEEPROMStorageModule;
    bool failureOfRealtimeClockModule;
    bool failureOfPrechargeModule;
    bool failureOfVehicleCommunicationModule;
    bool failureOfIntranetCommunicationModule;
    // 0x06: прочие
    bool failureOfCurrentSensorModule;
    bool failureOfMainVoltageSensorModule;
    bool failureOfShortCircuitProtection;
    bool failureOfLowVoltageNoCharging;

    // Сравнивает два состояния на равенство. Читает поля только через оператор.
    bool operator==(const BmsReadings& other) const {
        bool equal = true;

        equal &= (packVoltage == other.packVoltage);
        equal &= (packCurrent == other.packCurrent);
        equal &= (packSOC == other.packSOC);

        equal &= (maxCellmV == other.maxCellmV);
        equal &= (maxCellVNum == other.maxCellVNum);
        equal &= (minCellmV == other.minCellmV);
        equal &= (minCellVNum == other.minCellVNum);
        equal &= (cellDiff == other.cellDiff);

        equal &= (tempMax == other.tempMax);
        equal &= (tempMin == other.tempMin);
        equal &= (tempAverage == other.tempAverage);

        equal &= (chargeDischargeStatus == other.chargeDischargeStatus);
        equal &= (chargeFetState == other.chargeFetState);
        equal &= (disChargeFetState == other.disChargeFetState);
        equal &= (bmsHeartBeat == other.bmsHeartBeat);
        equal &= (resCapacitymAh == other.resCapacitymAh);

        equal &= (numberOfCells == other.numberOfCells);
        equal &= (numOfTempSensors == other.numOfTempSensors);
        equal &= (chargeState == other.chargeState);
        equal &= (loadState == other.loadState);
        for (uint8_t i = 0; i < 8u; ++i) {
            equal &= (dIO[i] == other.dIO[i]);
        }
        equal &= (bmsCycles == other.bmsCycles);

        for (uint8_t i = 0; i < MAX_NUMBER_CELLS; ++i) {
            equal &= (cellVmV[i] == other.cellVmV[i]);
        }
        for (uint8_t i = 0; i < MAX_NUMBER_TEMP_SENSORS; ++i) {
            equal &= (cellTemperature[i] == other.cellTemperature[i]);
        }
        for (uint8_t i = 0; i < MAX_NUMBER_CELLS; ++i) {
            equal &= (cellBalanceState[i] == other.cellBalanceState[i]);
        }
        equal &= (cellBalanceActive == other.cellBalanceActive);

        equal &= (levelOneCellVoltageTooHigh == other.levelOneCellVoltageTooHigh);
        equal &= (levelTwoCellVoltageTooHigh == other.levelTwoCellVoltageTooHigh);
        equal &= (levelOneCellVoltageTooLow == other.levelOneCellVoltageTooLow);
        equal &= (levelTwoCellVoltageTooLow == other.levelTwoCellVoltageTooLow);
        equal &= (levelOnePackVoltageTooHigh == other.levelOnePackVoltageTooHigh);
        equal &= (levelTwoPackVoltageTooHigh == other.levelTwoPackVoltageTooHigh);
        equal &= (levelOnePackVoltageTooLow == other.levelOnePackVoltageTooLow);
        equal &= (levelTwoPackVoltageTooLow == other.levelTwoPackVoltageTooLow);

        equal &= (levelOneChargeTempTooHigh == other.levelOneChargeTempTooHigh);
        equal &= (levelTwoChargeTempTooHigh == other.levelTwoChargeTempTooHigh);
        equal &= (levelOneChargeTempTooLow == other.levelOneChargeTempTooLow);
        equal &= (levelTwoChargeTempTooLow == other.levelTwoChargeTempTooLow);
        equal &= (levelOneDischargeTempTooHigh == other.levelOneDischargeTempTooHigh);
        equal &= (levelTwoDischargeTempTooHigh == other.levelTwoDischargeTempTooHigh);
        equal &= (levelOneDischargeTempTooLow == other.levelOneDischargeTempTooLow);
        equal &= (levelTwoDischargeTempTooLow == other.levelTwoDischargeTempTooLow);

        equal &= (levelOneChargeCurrentTooHigh == other.levelOneChargeCurrentTooHigh);
        equal &= (levelTwoChargeCurrentTooHigh == other.levelTwoChargeCurrentTooHigh);
        equal &= (levelOneDischargeCurrentTooHigh == other.levelOneDischargeCurrentTooHigh);
        equal &= (levelTwoDischargeCurrentTooHigh == other.levelTwoDischargeCurrentTooHigh);
        equal &= (levelOneStateOfChargeTooHigh == other.levelOneStateOfChargeTooHigh);
        equal &= (levelTwoStateOfChargeTooHigh == other.levelTwoStateOfChargeTooHigh);
        equal &= (levelOneStateOfChargeTooLow == other.levelOneStateOfChargeTooLow);
        equal &= (levelTwoStateOfChargeTooLow == other.levelTwoStateOfChargeTooLow);

        equal &= (levelOneCellVoltageDifferenceTooHigh == other.levelOneCellVoltageDifferenceTooHigh);
        equal &= (levelTwoCellVoltageDifferenceTooHigh == other.levelTwoCellVoltageDifferenceTooHigh);
        equal &= (levelOneTempSensorDifferenceTooHigh == other.levelOneTempSensorDifferenceTooHigh);
        equal &= (levelTwoTempSensorDifferenceTooHigh == other.levelTwoTempSensorDifferenceTooHigh);

        equal &= (chargeFETTemperatureTooHigh == other.chargeFETTemperatureTooHigh);
        equal &= (dischargeFETTemperatureTooHigh == other.dischargeFETTemperatureTooHigh);
        equal &= (failureOfChargeFETTemperatureSensor == other.failureOfChargeFETTemperatureSensor);
        equal &= (failureOfDischargeFETTemperatureSensor == other.failureOfDischargeFETTemperatureSensor);
        equal &= (failureOfChargeFETAdhesion == other.failureOfChargeFETAdhesion);
        equal &= (failureOfDischargeFETAdhesion == other.failureOfDischargeFETAdhesion);
        equal &= (failureOfChargeFETBreaker == other.failureOfChargeFETBreaker);
        equal &= (failureOfDischargeFETBreaker == other.failureOfDischargeFETBreaker);

        equal &= (failureOfAFEAcquisitionModule == other.failureOfAFEAcquisitionModule);
        equal &= (failureOfVoltageSensorModule == other.failureOfVoltageSensorModule);
        equal &= (failureOfTemperatureSensorModule == other.failureOfTemperatureSensorModule);
        equal &= (failureOfEEPROMStorageModule == other.failureOfEEPROMStorageModule);
        equal &= (failureOfRealtimeClockModule == other.failureOfRealtimeClockModule);
        equal &= (failureOfPrechargeModule == other.failureOfPrechargeModule);
        equal &= (failureOfVehicleCommunicationModule == other.failureOfVehicleCommunicationModule);
        equal &= (failureOfIntranetCommunicationModule == other.failureOfIntranetCommunicationModule);

        equal &= (failureOfCurrentSensorModule == other.failureOfCurrentSensorModule);
        equal &= (failureOfMainVoltageSensorModule == other.failureOfMainVoltageSensorModule);
        equal &= (failureOfShortCircuitProtection == other.failureOfShortCircuitProtection);
        equal &= (failureOfLowVoltageNoCharging == other.failureOfLowVoltageNoCharging);

        return equal;
    }
};

// Обёртка над драйвером Daly BMS: хранит инстанс библиотеки и предоставляет доступ к показаниям.
// Состояние БМС — структура BmsReadings; её считывают/обновляют через getState()/setState().
// Управляемой аппаратной части у BmsReader нет (БМС — внешнее устройство-источник), поэтому
// setState() только фиксирует состояние. Дополнительно отслеживается флаг ошибки связи:
// БМС не отвечает по UART, если батарея разряжена и БМС отключилась.
class BmsReader {
public:
    // Конструктор: привязывает драйвер к Serial (единственный UART у ATmega328P).
    BmsReader();

    // Инициализирует последовательный порт (9600, 8N1) для связи с БМС.
    void init();

    // Считывает все данные БМС в выходной параметр; при неудаче проставляет флаг ошибки
    // связи, при успехе сбрасывает его. Возвращает true при успешном чтении.
    bool readBmsValues(BmsReadings& readings);

    // True, если последнее чтение не удалось (БМС не отвечает по UART).
    bool isConnectionError() const;

    // Возвращает текущее состояние БМС в выходной параметр.
    void getState(BmsReadings& out) const;

    // Обновляет состояние БМС; управляемой аппаратной части нет (внешний источник).
    void setState(const BmsReadings& newState);

private:
    // Копирует структуры get/alarm библиотеки в выходной параметр.
    void copyFromLibrary(BmsReadings& readings) const;

    // Инстанс драйвера Daly BMS, подключённый к Serial.
    Daly_BMS_UART bms;

    // Состояние БМС (телеметрия и тревоги).
    BmsReadings readings;

    // Флаг ошибки связи с БМС: true — БМС не отвечает по UART (батарея разряжена/отключилась).
    bool connectionError;
};
