#pragma once

#include "bms_reader.h"
#include "charge_percentage_calculator.h"

// Состояние батареи: показания БМС и расчётный процент зарядки.
struct BatteryState {
    // Ток сборки, А (положительный — заряд, отрицательный — разряд).
    float current;

    // Напряжение сборки, В.
    float voltage;

    // Флаг нормальной температуры батареи по тревогам БМС.
    bool isTemperatureOk;

    // Расчётный оставшийся процент зарядки, % (0..100).
    float chargePercentage;

    // Сравнивает два состояния на равенство.
    bool operator==(const BatteryState& other) const {
        return current == other.current && voltage == other.voltage &&
               isTemperatureOk == other.isTemperatureOk &&
               chargePercentage == other.chargePercentage;
    }
};

// Читатель батареи: опрашивает БМС через BmsReader (ток, напряжение,
// температурные тревоги) и рассчитывает оставшийся процент зарядки по
// кривой разряда LiFePO4. Батарея — источник показаний, управляемой
// аппаратной части у читателя нет: setState() отсутствует, точка входа
// для чтения задаёт update() (как у BmsReader).
class BatteryReader {
public:
    // Конструктор: принимает читатель БМС — единственного владельца
    // телеметрии Daly (критично для 2 КБ RAM ATmega328P).
    explicit BatteryReader(BmsReader& bms);

    // Опрашивает БМС и обновляет состояние: при успехе заполняет ток,
    // напряжение, флаг температуры и пересчитывает процент зарядки.
    // Возвращает true при успешном чтении; при ошибке связи состояние
    // остаётся с прошлого успешного чтения.
    bool update();

    // Считывает последнее зафиксированное состояние батареи.
    void getState(BatteryState& out);

private:
    // Читатель телеметрии Daly BMS.
    BmsReader& bms;

    // Калькулятор оставшегося процента зарядки по напряжению.
    ChargePercentageCalculator percentage_calculator;

    // Последнее зафиксированное состояние батареи.
    BatteryState state;
};
