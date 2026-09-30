#include "charge_percentage_calculator.h"

#include <mlinterp/mlinterp.hpp>

#include "charge_curve_table.h"

// Число точек кривой для вызова интерполяции (тип размера индекса).
static const int CURVE_POINTS = CHARGE_CURVE_POINTS;

float ChargePercentageCalculator::calculate(float packVoltage) {
  // Отсутствие значения (NaN из телеметрии БМС): батарею считаем пустой.
  // Проверка «x != x» истинна только для NaN и не тянет зависимостей.
  if (packVoltage != packVoltage) {
    return 0.0f;
  }

  // Кривая разряда построена для одной ячейки: пересчёт напряжения
  // сборки 8s в напряжение ячейки.
  float cellVoltage = packVoltage / NUM_CELLS;

  // Линейная интерполяция процента зарядки по напряжению ячейки;
  // mlinterp ограничивает результат краями таблицы за её пределами
  // (ниже минимума — 0 %, выше максимума — 100 %).
  float chargePercentage = 0.0f;
  mlinterp::interp(&CURVE_POINTS, 1, CHARGE_CURVE_PERCENT, &chargePercentage,
                   CHARGE_CURVE_CELL_VOLTAGE, &cellVoltage);
  return chargePercentage;
}
