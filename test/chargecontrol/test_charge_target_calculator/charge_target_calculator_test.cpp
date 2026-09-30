// Тесты подбора включаемых зарядок (свободная функция calculateTargetState).
//
// Номиналы: зарядка 1 — 25 А, зарядка 2 — 25 А, зарядка 3 — 15 А,
// зарядка 4 — резерв (расчётом не включается никогда). Запас минимального
// тока — 5 А.
//
//   | Вход                          | Ожидание activeRelays / isChargeOn   |
//   |-------------------------------|--------------------------------------|
//   | перегрев                      | 0000, off                            |
//   | minimal=false, ток 30 А       | 1110, on (все реальные зарядки)      |
//   | minimal=false, ток -5 А       | 1110, on (внешняя зарядка)           |
//   | minimal=true, ток 30 А        | 1010 (25+15=40, а не 25+25=50)       |
//   | minimal=true, ток 20 А        | 1000 (25 >= 25)                      |
//   | minimal=true, ток 39 А        | 1100 (50 — минимум из >= 44)         |
//   | minimal=true, ток 60 А        | 1110 (65 >= 65)                      |
//   | minimal=true, ток 100 А       | 1110 (недостижимо — все зарядки)     |
//   | minimal=true, ток 0 А         | 0010 (15 — минимум из >= 5)          |
//   | minimal=true, разряд -20 А    | 0000, off (порог отрицательный)      |

#include <unity.h>

#include "chargecontrol/charge_control_state.h"
#include "chargecontrol/charge_target_calculator.h"

// Вход и выход расчёта — статические объекты (правило «Память»).
static BatteryState input;
static ChargeControlState out;

void setUp() {
  input.current = 0.0f;
  input.voltage = 0.0f;
  input.isTemperatureOk = true;
  input.chargePercentage = 0.0f;
}

void tearDown() {}

// Проверяет состояние зарядок: флаг isChargeOn и флаги активных реле.
static void assertState(bool chargeOn, uint8_t r1, uint8_t r2, uint8_t r3, uint8_t r4) {
  TEST_ASSERT_EQUAL_UINT8(chargeOn ? 1 : 0, out.isChargeOn ? 1 : 0);
  TEST_ASSERT_EQUAL_UINT8(r1, out.activeRelays[0]);
  TEST_ASSERT_EQUAL_UINT8(r2, out.activeRelays[1]);
  TEST_ASSERT_EQUAL_UINT8(r3, out.activeRelays[2]);
  TEST_ASSERT_EQUAL_UINT8(r4, out.activeRelays[3]);
}

// Перегрев: зарядка запрещена при любых флагах и токе.
void test_overheat_disables_everything() {
  input.isTemperatureOk = false;
  input.current = 30.0f;

  calculateTargetState(input, out, false);
  assertState(false, 0, 0, 0, 0);

  calculateTargetState(input, out, true);
  assertState(false, 0, 0, 0, 0);
}

// Полная мощность: при нормальной температуре включаются все реальные
// зарядки; резервная (четвёртая) не включается никогда.
void test_full_power_turns_on_all_chargers() {
  input.current = 30.0f;
  calculateTargetState(input, out, false);
  assertState(true, 1, 1, 1, 0);
}

// Полная мощность при токе <= 0: идёт зарядка от внешней зарядки, которой
// нет в списке реле, — наши зарядки всё равно включаются.
void test_full_power_negative_current_keeps_chargers_on() {
  input.current = -5.0f;
  calculateTargetState(input, out, false);
  assertState(true, 1, 1, 1, 0);
}

// Ток 30 А: минимальная достаточная сумма — 25 + 15 = 40 А, а не 50 А.
void test_minimal_current_30a_picks_25_plus_15() {
  input.current = 30.0f;
  calculateTargetState(input, out, true);
  assertState(true, 1, 0, 1, 0);
}

// Ток 20 А: порог 25 А покрывается одной зарядкой на 25 А.
void test_minimal_current_20a_picks_single_25() {
  input.current = 20.0f;
  calculateTargetState(input, out, true);
  assertState(true, 1, 0, 0, 0);
}

// Ток 39 А: порог 44 А; кандидаты 50 А (1+2) и 65 А — выбирается 50 А.
void test_minimal_current_39a_picks_two_25() {
  input.current = 39.0f;
  calculateTargetState(input, out, true);
  assertState(true, 1, 1, 0, 0);
}

// Ток 60 А: порог 65 А в точности покрывается всеми тремя зарядками.
void test_minimal_current_60a_picks_all() {
  input.current = 60.0f;
  calculateTargetState(input, out, true);
  assertState(true, 1, 1, 1, 0);
}

// Ток 100 А: порог недостижим даже всеми зарядками — включаем всё
// доступное.
void test_minimal_current_unreachable_picks_all() {
  input.current = 100.0f;
  calculateTargetState(input, out, true);
  assertState(true, 1, 1, 1, 0);
}

// Нулевой ток: порог 5 А, минимальная достаточная сумма — 15 А (зарядка 3).
void test_minimal_current_zero_picks_15() {
  input.current = 0.0f;
  calculateTargetState(input, out, true);
  assertState(true, 0, 0, 1, 0);
}

// Разряд (ток отрицательный): порог отрицательный, достаточна пустая
// комбинация — зарядки выключены.
void test_minimal_current_discharge_turns_off() {
  input.current = -20.0f;
  calculateTargetState(input, out, true);
  assertState(false, 0, 0, 0, 0);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_overheat_disables_everything);
  RUN_TEST(test_full_power_turns_on_all_chargers);
  RUN_TEST(test_full_power_negative_current_keeps_chargers_on);
  RUN_TEST(test_minimal_current_30a_picks_25_plus_15);
  RUN_TEST(test_minimal_current_20a_picks_single_25);
  RUN_TEST(test_minimal_current_39a_picks_two_25);
  RUN_TEST(test_minimal_current_60a_picks_all);
  RUN_TEST(test_minimal_current_unreachable_picks_all);
  RUN_TEST(test_minimal_current_zero_picks_15);
  RUN_TEST(test_minimal_current_discharge_turns_off);
  return UNITY_END();
}
