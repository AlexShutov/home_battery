#include "ir_sensor.h"
#include <math.h>

IrSensor::IrSensor() {
  state.address = 0;
  state.temperature = NAN;
  state.connected = false;
}

bool IrSensor::init(uint8_t address) {
  state.address = address;
  state.temperature = NAN;
  state.connected = false;

  // Подключение к датчику по адресу (как в TestIRSensor).
  if (!sensor.begin(address)) {
    return false;
  }

  // Контрольное чтение: посторонний I2C-девайс вместо MLX90614 даёт NAN.
  state.temperature = sensor.readObjectTempC();
  state.connected = !isnan(state.temperature);
  return state.connected;
}

float IrSensor::read() {
  // Неподключённый датчик читать нельзя: внутри библиотеки указатель шины пуст.
  if (!state.connected) {
    state.temperature = NAN;
    return state.temperature;
  }
  state.temperature = sensor.readObjectTempC();
  return state.temperature;
}

void IrSensor::getState(IrSensorState& out) const {
  out = state;
}

void IrSensor::setState(const IrSensorState& newState) {
  // Датчик — только источник данных, управляемой аппаратной части нет.
  state = newState;
}
