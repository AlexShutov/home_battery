#pragma once

// Состояние батареи: показания БМС и расчётный процент зарядки. Чистые
// данные без зависимостей от Arduino — используются и в прошивке, и в
// нативных юнит-тестах.
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
