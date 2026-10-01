// Тесты стейт-машины зарядки: таблицы переходов по тарифам и логики
// недавно изменённых состояний (AlwaysChargingState, MinimalPowerChargingState).
//
// Механика: исходники машины и контроля зарядок компилируются прямо в
// тестовое приложение (LDF не сканирует test/), регистры порта D AVR —
// переменные в памяти (см. Arduino.h рядом). Свободная функция расчёта
// зарядок calculateTargetState уже собирается глобально (build_src_filter),
// поэтому её исходник сюда не включаем.
//
// Покрытие:
//
//   | Случай                                                   | Ожидание              |
//   |----------------------------------------------------------|-----------------------|
//   | старт машины                                             | режим OFF             |
//   | CHEAP/MIDDLE/EXPENSIVE/FORCE_CHARGING                    | MAX/MINIMAL/OFF/ALWAYS|
//   | повторный тариф того же значения                         | без выхода из стейта  |
//   | вход в ALWAYS при нормальной температуре                 | зарядки 1110 (ножки)  |
//   | вход в ALWAYS при перегреве                              | зарядки 0000          |
//   | выход из ALWAYS в CHEAP                                  | зарядки не гаснут     |
//   | вход в MIDDLE, заряд > 60%                               | зарядки 0000          |
//   | вход в MIDDLE, заряд < 50%                               | зарядки 1110, флаг    |
//   | вход в MIDDLE, заряд 50–60%                              | прошлое сохраняется   |
//   | флаг поднят, заряд вырос > 60% + событие батареи         | зарядки остаются вкл  |
//   | флаг поднят, перегрев + событие батареи                  | зарядки 0000          |
//   | выход из MIDDLE сбрасывает флаг                          | повторный вход: 0000  |
//   | OFF игнорирует событие батареи                           | зарядки не меняются   |

#include "Arduino.h"

#include <unity.h>

// Исходники стейт-машины и контроля зарядок — в состав тестового
// приложения. Заглушка Arduino.h выше закрывает зависимости от каркаса.
#include "chargecontrol/charge_control.cpp"
#include "chargecontrol/relays.cpp"
#include "device/statemachine/always_charging_state.cpp"
#include "device/statemachine/charging_off_state.cpp"
#include "device/statemachine/device_state_machine.cpp"
#include "device/statemachine/max_power_charging_state.cpp"
#include "device/statemachine/minimal_power_charging_state.cpp"
#include "device/statemachine/overheating_state.cpp"

// Регистры порта D AVR — переменные в памяти (см. Arduino.h).
uint8_t DDRD;
uint8_t PORTD;

// Тестируемый контроль зарядок и буферы состояний — статические объекты
// (правило «Память»).
static ChargeControl charge_control_under_test;
static BatteryState battery_under_test;
static RelayState relays_under_test;

// Базовый снимок батареи: нормальная температура, ток 30 А, заряд 55%.
static void resetBattery() {
  battery_under_test.current = 30.0f;
  battery_under_test.voltage = 53.0f;
  battery_under_test.isTemperatureOk = true;
  battery_under_test.chargePercentage = 55.0f;
}

void setUp() {
  DDRD = 0;
  PORTD = 0;
  charge_control_under_test.init();
  bindChargeControl(charge_control_under_test);
  // Сбрасывает внутренние флаги всех состояний машины: tinyfsm хранит
  // каждое состояние как статический синглтон, а start() не вызывает
  // exit() предыдущего состояния (перезапуск сменяет только указатель на
  // текущее состояние). Без сброса флаг isHighConsumption состояния MIDDLE
  // переживал бы setUp() и ломал бы изоляцию тестов.
  DeviceStateMachine::state<ChargingOffState>() = ChargingOffState();
  DeviceStateMachine::state<MinimalPowerChargingState>() = MinimalPowerChargingState();
  DeviceStateMachine::state<MaxPowerChargingState>() = MaxPowerChargingState();
  DeviceStateMachine::state<OverheatingState>() = OverheatingState();
  DeviceStateMachine::state<AlwaysChargingState>() = AlwaysChargingState();
  resetBattery();
  setBatterySnapshot(battery_under_test);
  // Старт (или перезапуск) машины: вход в начальное состояние ChargingOff.
  // Если тест оставил машину в другом состоянии, произойдёт выход из него
  // (например, сброс флага isHighConsumption состояния MIDDLE).
  startDeviceStateMachine();
}

void tearDown() {}

// Проверяет логическое состояние четырёх реле зарядок.
static void assertRelays(bool r1, bool r2, bool r3, bool r4) {
  charge_control_under_test.getRelayState(relays_under_test);
  TEST_ASSERT_TRUE(r1 ? relays_under_test.relays[0] : !relays_under_test.relays[0]);
  TEST_ASSERT_TRUE(r2 ? relays_under_test.relays[1] : !relays_under_test.relays[1]);
  TEST_ASSERT_TRUE(r3 ? relays_under_test.relays[2] : !relays_under_test.relays[2]);
  TEST_ASSERT_TRUE(r4 ? relays_under_test.relays[3] : !relays_under_test.relays[3]);
}

// После старта машина находится в состоянии «зарядка выключена».
void test_initial_state_is_charging_off() {
  TEST_ASSERT_EQUAL_INT(CHARGING_MODE_OFF, getChargingMode());
}

// Таблица переходов: тариф переводит машину в соответствующее состояние.
void test_tariff_table_transitions() {
  setTariff(CHEAP);
  TEST_ASSERT_EQUAL_INT(CHARGING_MODE_MAX, getChargingMode());

  setTariff(MIDDLE);
  TEST_ASSERT_EQUAL_INT(CHARGING_MODE_MINIMAL, getChargingMode());

  setTariff(EXPENSIVE);
  TEST_ASSERT_EQUAL_INT(CHARGING_MODE_OFF, getChargingMode());

  setTariff(FORCE_CHARGING);
  TEST_ASSERT_EQUAL_INT(CHARGING_MODE_ALWAYS, getChargingMode());
}

// Повторное событие с тем же тарифом не выходит из состояния: флаг
// isHighConsumption состояния MIDDLE не сбрасывается (сброс — только
// через exit). Сценарий: 45% -> флаг поднят; повторный MIDDLE; событие
// батареи с зарядом 90% — зарядки остаются включёнными.
void test_repeated_same_tariff_keeps_state() {
  battery_under_test.chargePercentage = 45.0f;
  setBatterySnapshot(battery_under_test);
  setTariff(MIDDLE);
  assertRelays(true, true, true, false);

  setTariff(MIDDLE); // без выхода из состояния

  battery_under_test.chargePercentage = 90.0f;
  setBatterySnapshot(battery_under_test);
  notifyBatterySnapshot();
  assertRelays(true, true, true, false);
}

// Вход в ALWAYS при нормальной температуре: включаются все реальные
// зарядки (четвёртая — резерв), ножки порта D переключаются (реле 1–3:
// биты 2–4 сброшены, резервный бит 5 остаётся высоким).
void test_always_charging_turns_on_all_chargers() {
  setTariff(FORCE_CHARGING);
  TEST_ASSERT_EQUAL_INT(CHARGING_MODE_ALWAYS, getChargingMode());
  assertRelays(true, true, true, false);
  TEST_ASSERT_EQUAL_UINT8(0x20, PORTD);
}

// Вход в ALWAYS при перегреве: activateTargetState выключает всё.
void test_always_charging_overheat_disables_all() {
  battery_under_test.isTemperatureOk = false;
  setBatterySnapshot(battery_under_test);
  setTariff(FORCE_CHARGING);
  assertRelays(false, false, false, false);
}

// Выход из ALWAYS не выключает зарядки: новое состояние применяет свою
// конфигурацию само (MaxPower — пока заглушка, зарядки не трогает).
void test_always_charging_exit_keeps_chargers() {
  setTariff(FORCE_CHARGING);
  assertRelays(true, true, true, false);

  setTariff(CHEAP);
  TEST_ASSERT_EQUAL_INT(CHARGING_MODE_MAX, getChargingMode());
  assertRelays(true, true, true, false);
}

// Вход в MIDDLE с зарядом выше 60%: пользователю хватит — зарядки выключены.
void test_middle_entry_high_charge_disables() {
  battery_under_test.chargePercentage = 70.0f;
  setBatterySnapshot(battery_under_test);
  setTariff(MIDDLE);
  assertRelays(false, false, false, false);
}

// Вход в MIDDLE с зарядом ниже 50%: высокое потребление — все зарядки.
void test_middle_entry_low_charge_enables_all() {
  battery_under_test.chargePercentage = 45.0f;
  setBatterySnapshot(battery_under_test);
  setTariff(MIDDLE);
  assertRelays(true, true, true, false);
}

// Зона гистерезиса 50–60% при входе: конфигурация зарядок не меняется —
// и после выключенных (OFF), и после включённых (ALWAYS) зарядок.
void test_middle_entry_hysteresis_zone_keeps_previous() {
  battery_under_test.chargePercentage = 55.0f;
  setBatterySnapshot(battery_under_test);
  setTariff(MIDDLE); // из ChargingOff: зарядки были выключены
  assertRelays(false, false, false, false);

  setTariff(FORCE_CHARGING); // зарядки включены
  setTariff(MIDDLE);         // снова вход, заряд 55%
  assertRelays(true, true, true, false);
}

// Флаг isHighConsumption держится до выхода из состояния: выросший выше
// 60% заряд не выключает дозарядку.
void test_middle_flag_survives_high_charge() {
  battery_under_test.chargePercentage = 45.0f;
  setBatterySnapshot(battery_under_test);
  setTariff(MIDDLE);
  assertRelays(true, true, true, false);

  battery_under_test.chargePercentage = 90.0f;
  setBatterySnapshot(battery_under_test);
  notifyBatterySnapshot();
  assertRelays(true, true, true, false);
}

// Перегрев главнее флага дозарядки: событие батареи с перегревом
// выключает зарядки даже при isHighConsumption = true.
void test_middle_overheat_disables_even_with_flag() {
  battery_under_test.chargePercentage = 45.0f;
  setBatterySnapshot(battery_under_test);
  setTariff(MIDDLE);
  assertRelays(true, true, true, false);

  battery_under_test.isTemperatureOk = false;
  setBatterySnapshot(battery_under_test);
  notifyBatterySnapshot();
  assertRelays(false, false, false, false);
}

// Выход из MIDDLE сбрасывает флаг: повторный вход при высоком заряде
// выключает зарядки (если бы флаг пережил выход — они остались бы вкл).
void test_middle_flag_resets_on_exit() {
  battery_under_test.chargePercentage = 45.0f;
  setBatterySnapshot(battery_under_test);
  setTariff(MIDDLE);
  assertRelays(true, true, true, false);

  setTariff(EXPENSIVE); // выход из MIDDLE: флаг сброшен

  battery_under_test.chargePercentage = 90.0f;
  setBatterySnapshot(battery_under_test);
  setTariff(MIDDLE); // флаг снят, заряд > 60% — зарядки выключены
  assertRelays(false, false, false, false);
}

// Состояние ChargingOff игнорирует событие обновления батареи: даже
// перегрев не меняет конфигурацию зарядок (зарядки после ALWAYS включены,
// заглушка OFF их не гасит — логика будет добавлена в ChargingOffState).
void test_off_ignores_battery_events() {
  setTariff(FORCE_CHARGING);
  assertRelays(true, true, true, false);

  setTariff(EXPENSIVE);
  TEST_ASSERT_EQUAL_INT(CHARGING_MODE_OFF, getChargingMode());

  battery_under_test.isTemperatureOk = false;
  setBatterySnapshot(battery_under_test);
  notifyBatterySnapshot();
  assertRelays(true, true, true, false);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_initial_state_is_charging_off);
  RUN_TEST(test_tariff_table_transitions);
  RUN_TEST(test_repeated_same_tariff_keeps_state);
  RUN_TEST(test_always_charging_turns_on_all_chargers);
  RUN_TEST(test_always_charging_overheat_disables_all);
  RUN_TEST(test_always_charging_exit_keeps_chargers);
  RUN_TEST(test_middle_entry_high_charge_disables);
  RUN_TEST(test_middle_entry_low_charge_enables_all);
  RUN_TEST(test_middle_entry_hysteresis_zone_keeps_previous);
  RUN_TEST(test_middle_flag_survives_high_charge);
  RUN_TEST(test_middle_overheat_disables_even_with_flag);
  RUN_TEST(test_middle_flag_resets_on_exit);
  RUN_TEST(test_off_ignores_battery_events);
  return UNITY_END();
}
