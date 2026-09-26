#include "temperature_control.h"
#include <math.h>

const int8_t TemperatureControl::MIN_THRESHOLD_TEMPS[TemperatureControl::NUM_SENSORS] = {
    SENSOR_1_MIN_THRESHOLD_TEMP,
    SENSOR_2_MIN_THRESHOLD_TEMP,
    SENSOR_3_MIN_THRESHOLD_TEMP,
    SENSOR_4_MIN_THRESHOLD_TEMP,
};

const int8_t TemperatureControl::MAX_THRESHOLD_TEMPS[TemperatureControl::NUM_SENSORS] = {
    SENSOR_1_MAX_THRESHOLD_TEMP,
    SENSOR_2_MAX_THRESHOLD_TEMP,
    SENSOR_3_MAX_THRESHOLD_TEMP,
    SENSOR_4_MAX_THRESHOLD_TEMP,
};

TemperatureControl::TemperatureControl() {
  onCooledDown = nullptr;
  onTempTooHigh = nullptr;
  for (uint8_t i = 0; i < NUM_SENSORS; ++i) {
    state.temperatures[i] = NAN;
    state.outOfRange[i] = false;
  }
  state.allCooledDown = false;
  state.allTooHot = false;
}

// Инициализирует датчик по индексу с заданным адресом; IrSensor::init читает
// одно контрольное показание и возвращает true только при корректном значении.
bool TemperatureControl::initSensor(uint8_t index, uint8_t address) {
  state.temperatures[index] = NAN;
  state.outOfRange[index] = false;
  return sensors[index].init(address);
}

bool TemperatureControl::init(TemperatureCallback cooledDown, TemperatureCallback tooHigh) {
  onCooledDown = cooledDown;
  onTempTooHigh = tooHigh;

  // Подключаем все датчики по их константным адресам (отсутствующий датчик
  // останется с NAN — loop() это корректно обработает). Инициализируем все
  // подряд, независимо от результата предыдущего.
  bool allConnected = initSensor(0, SENSOR_1_ADDRESS);
  allConnected = initSensor(1, SENSOR_2_ADDRESS) && allConnected;
  allConnected = initSensor(2, SENSOR_3_ADDRESS) && allConnected;
  allConnected = initSensor(3, SENSOR_4_ADDRESS) && allConnected;

  state.allCooledDown = false;
  state.allTooHot = false;

  // true — все датчики ответили корректным показанием (не NAN).
  return allConnected;
}

void TemperatureControl::loop() {
  bool belowMin = true;
  bool aboveMax = true;

  for (uint8_t i = 0; i < NUM_SENSORS; ++i) {
    state.temperatures[i] = sensors[i].read();
    if (isnan(state.temperatures[i])) {
      // Данных нет: агрегированные состояния неопределённы, колбэки не вызываем.
      belowMin = false;
      aboveMax = false;
      state.outOfRange[i] = false;
    } else {
      // Показание за пределами порогов данного датчика.
      state.outOfRange[i] = state.temperatures[i] < (float)MIN_THRESHOLD_TEMPS[i] ||
                            state.temperatures[i] > (float)MAX_THRESHOLD_TEMPS[i];
      belowMin = belowMin && state.temperatures[i] < (float)MIN_THRESHOLD_TEMPS[i];
      aboveMax = aboveMax && state.temperatures[i] > (float)MAX_THRESHOLD_TEMPS[i];
    }
  }

  // Колбэки вызываются в момент перехода в состояние, а не на каждом цикле.
  if (belowMin && !state.allCooledDown && onCooledDown != nullptr) {
    onCooledDown();
  }
  if (aboveMax && !state.allTooHot && onTempTooHigh != nullptr) {
    onTempTooHigh();
  }

  state.allCooledDown = belowMin;
  state.allTooHot = aboveMax;
}

void TemperatureControl::getState(TemperatureControlState& out) const {
  out = state;
}

void TemperatureControl::setState(const TemperatureControlState& newState) {
  // Управляемой аппаратной части нет: датчики — только источники показаний.
  state = newState;
}
