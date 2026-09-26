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
  onSensorInitFailed = nullptr;
  ignoreFailedSensors = false;
  for (uint8_t i = 0; i < NUM_SENSORS; ++i) {
    state.temperatures[i] = NAN;
    state.connected[i] = false;
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
  state.connected[index] = sensors[index].init(address);
  return state.connected[index];
}

bool TemperatureControl::init(TemperatureCallback cooledDown, TemperatureCallback tooHigh,
                              SensorInitFailedCallback sensorInitFailed, bool ignoreFailed) {
  onCooledDown = cooledDown;
  onTempTooHigh = tooHigh;
  onSensorInitFailed = sensorInitFailed;
  ignoreFailedSensors = ignoreFailed;

  // Подключаем все датчики по их константным адресам (отсутствующий датчик
  // останется с NAN — loop() это корректно обработает). Инициализируем все
  // подряд; при сбое сообщаем номер датчика, если режим игнорирования выключен.
  bool allConnected = initSensor(0, SENSOR_1_ADDRESS);
  if (!allConnected && !ignoreFailedSensors && onSensorInitFailed != nullptr) {
    onSensorInitFailed(1);
  }
  bool sensor2 = initSensor(1, SENSOR_2_ADDRESS);
  if (!sensor2 && !ignoreFailedSensors && onSensorInitFailed != nullptr) {
    onSensorInitFailed(2);
  }
  bool sensor3 = initSensor(2, SENSOR_3_ADDRESS);
  if (!sensor3 && !ignoreFailedSensors && onSensorInitFailed != nullptr) {
    onSensorInitFailed(3);
  }
  bool sensor4 = initSensor(3, SENSOR_4_ADDRESS);
  if (!sensor4 && !ignoreFailedSensors && onSensorInitFailed != nullptr) {
    onSensorInitFailed(4);
  }
  allConnected = allConnected && sensor2 && sensor3 && sensor4;

  state.allCooledDown = false;
  state.allTooHot = false;

  // true — все датчики ответили корректным показанием (не NAN).
  return allConnected;
}

void TemperatureControl::loop() {
  bool belowMin = true;
  bool aboveMax = true;
  // Есть ли хотя бы одно валидное показание (страховка от ложного срабатывания
  // колбэков, когда в режиме игнорирования не ответил ни один датчик).
  bool anyValid = false;

  for (uint8_t i = 0; i < NUM_SENSORS; ++i) {
    state.temperatures[i] = sensors[i].read();
    if (isnan(state.temperatures[i])) {
      state.outOfRange[i] = false;
      if (!ignoreFailedSensors) {
        // Строгий режим: любые отсутствующие данные делают агрегаты неопределёнными.
        belowMin = false;
        aboveMax = false;
      }
      // В режиме игнорирования неработающий датчик исключается из проверки.
      continue;
    }

    anyValid = true;
    // Показание за пределами порогов данного датчика.
    state.outOfRange[i] = state.temperatures[i] < (float)MIN_THRESHOLD_TEMPS[i] ||
                          state.temperatures[i] > (float)MAX_THRESHOLD_TEMPS[i];
    belowMin = belowMin && state.temperatures[i] < (float)MIN_THRESHOLD_TEMPS[i];
    aboveMax = aboveMax && state.temperatures[i] > (float)MAX_THRESHOLD_TEMPS[i];
  }

  // Ни одного валидного показания — агрегаты неопределённы, колбэки не вызываем.
  if (!anyValid) {
    belowMin = false;
    aboveMax = false;
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
