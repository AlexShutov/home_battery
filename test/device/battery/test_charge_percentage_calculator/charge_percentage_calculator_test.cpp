#include <unity.h>

#include "device/battery/charge_curve_table.h"
#include "device/battery/charge_percentage_calculator.h"

// Тестируемый калькулятор: статический объект — по правилу «Память»
// объекты неинтегральных типов живут только в статическом хранилище.
static ChargePercentageCalculator calculator;

void setUp() {}

void tearDown() {}

// Полная сборка: максимум 29.2 В (3.65 В на ячейку) — 100 %.
void test_full_pack_is_100_percent() {
  TEST_ASSERT_FLOAT_WITHIN(0.05f, 100.0f,
                           calculator.calculate(ChargePercentageCalculator::MAX_PACK_VOLTAGE));
}

// Напряжение выше кривой (сразу после заряда) ограничивается 100 %.
void test_above_curve_clamps_to_100_percent() {
  TEST_ASSERT_FLOAT_WITHIN(0.05f, 100.0f, calculator.calculate(31.0f));
}

// Плато кривой: 26.0 В (3.25 В на ячейку) — середина заряда.
void test_plateau_26v_is_about_46_percent() {
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 45.9f, calculator.calculate(26.0f));
}

// Нижнее «колено» кривой: 25.0 В — около 9 %.
void test_knee_25v_is_about_9_percent() {
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 9.0f, calculator.calculate(25.0f));
}

// Разряженная сборка ниже минимума таблицы ограничивается 0 %.
void test_below_curve_clamps_to_0_percent() {
  TEST_ASSERT_FLOAT_WITHIN(0.05f, 0.0f, calculator.calculate(16.0f));
}

// Табличный узел 3.219 В (x8 ячеек = 25.752 В) — ровно 30 %.
void test_table_node_3_219v_is_30_percent() {
  TEST_ASSERT_FLOAT_WITHIN(0.05f, 30.0f, calculator.calculate(3.219f * 8.0f));
}

// Все узлы таблицы воспроизводятся точно: интерполяция в узле не
// искажает значение кривой.
void test_all_table_nodes_are_exact() {
  for (size_t i = 0; i < CHARGE_CURVE_POINTS; ++i) {
    float percent = calculator.calculate(CHARGE_CURVE_CELL_VOLTAGE[i] * 8.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, CHARGE_CURVE_PERCENT[i], percent);
  }
}

// Процент зарядки монотонно не убывает с ростом напряжения сборки.
void test_percent_is_monotonic_over_voltage() {
  float prev = -1.0f;
  for (size_t i = 0; i < CHARGE_CURVE_POINTS; ++i) {
    float percent = calculator.calculate(CHARGE_CURVE_CELL_VOLTAGE[i] * 8.0f);
    TEST_ASSERT_TRUE(percent >= prev);
    prev = percent;
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_full_pack_is_100_percent);
  RUN_TEST(test_above_curve_clamps_to_100_percent);
  RUN_TEST(test_plateau_26v_is_about_46_percent);
  RUN_TEST(test_knee_25v_is_about_9_percent);
  RUN_TEST(test_below_curve_clamps_to_0_percent);
  RUN_TEST(test_table_node_3_219v_is_30_percent);
  RUN_TEST(test_all_table_nodes_are_exact);
  RUN_TEST(test_percent_is_monotonic_over_voltage);
  return UNITY_END();
}
