#pragma once

#include <stdint.h>

// Калькулятор оставшегося процента зарядки LiFePO4-сборки по напряжению.
// Таблица кривой разряда (charge_curve_table.h) построена скриптом
// scripts/generate_charge_curve.py по научному датасету IEEE DataPort
// (DOI 10.21227/cm0f-jg66, CC BY 4.0): профиль C/3, среднее по 5 ячейкам.
class ChargePercentageCalculator {
public:
    // Рассчитывает оставшийся процент зарядки (0..100 %) по напряжению
    // сборки, В. Напряжение ячейки получается делением на число ячеек,
    // затем линейной интерполяцией (библиотека mlinterp) по кривой
    // разряда находится процент. Вне диапазона таблицы значение
    // ограничивается краями (0 % / 100 %).
    float calculate(float packVoltage);

    // Число последовательно соединённых ячеек (LiFePO4 8s).
    static const uint8_t NUM_CELLS = 8;

    // Максимальное напряжение сборки, В (3.65 В на ячейку).
    static constexpr float MAX_PACK_VOLTAGE = 29.2f;
};
