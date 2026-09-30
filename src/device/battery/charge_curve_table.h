// Сгенерировано скриптом scripts/generate_charge_curve.py — вручную не редактировать.
// Кривая разряда LiFePO4 (BSE 1500 мА*ч, C/3), среднее по 5 ячейкам.
// Источник данных: IEEE DataPort DOI 10.21227/cm0f-jg66 (CC BY 4.0).
#pragma once

#include <stddef.h>

// Число точек кривой.
static const size_t CHARGE_CURVE_POINTS = 21;

// Напряжение ячейки, В (по возрастанию).
static const float CHARGE_CURVE_CELL_VOLTAGE[CHARGE_CURVE_POINTS] = {
    2.5322f, 3.0418f, 3.1451f,
    3.1613f, 3.1866f, 3.2056f,
    3.2190f, 3.2310f, 3.2418f,
    3.2492f, 3.2536f, 3.2558f,
    3.2583f, 3.2600f, 3.2639f,
    3.2796f, 3.2866f, 3.2920f,
    3.2933f, 3.2969f, 3.3474f,
};

// Оставшийся процент зарядки, % (соответствует напряжению).
static const float CHARGE_CURVE_PERCENT[CHARGE_CURVE_POINTS] = {
    0.0f, 5.0f, 10.0f,
    15.0f, 20.0f, 25.0f,
    30.0f, 35.0f, 40.0f,
    45.0f, 50.0f, 55.0f,
    60.0f, 65.0f, 70.0f,
    75.0f, 80.0f, 85.0f,
    90.0f, 95.0f, 100.0f,
};
