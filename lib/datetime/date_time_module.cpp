#include "date_time_module.h"

DateTimeModule::DateTimeModule() {
  state.year = 2000;
  state.month = 1;
  state.day = 1;
  state.hour = 0;
  state.minute = 0;
  state.second = 0;
  state.connected = false;
}

bool DateTimeModule::init(DateTimeCallback onInitOk, DateTimeCallback onInitFailed) {
  // begin() ищет DS3231 на шине I2C (перед этим Wire уже инициализирован шиной
  // дисплея в DeviceLogic::init). lostPower() поднят, если резервная батарея
  // села и показание времени невалидно — такую инициализацию считаем неудачей.
  state.connected = rtc.begin() && !rtc.lostPower();

  if (state.connected) {
    if (onInitOk != nullptr) {
      onInitOk();
    }
  } else {
    if (onInitFailed != nullptr) {
      onInitFailed();
    }
  }

  return state.connected;
}

void DateTimeModule::getTime(DateTime& out) {
  // now() возвращает модель библиотеки по значению — переносим её в выходной
  // параметр (модель передаётся по ссылке, локальные объекты не создаются).
  out = rtc.now();

  // Обновляем интегральное состояние по полям модели.
  state.year = out.year();
  state.month = out.month();
  state.day = out.day();
  state.hour = out.hour();
  state.minute = out.minute();
  state.second = out.second();
}

void DateTimeModule::setTime(const DateTime& newTime) {
  // adjust() записывает время в DS3231.
  rtc.adjust(newTime);

  // Синхронизируем состояние с записанным значением.
  state.year = newTime.year();
  state.month = newTime.month();
  state.day = newTime.day();
  state.hour = newTime.hour();
  state.minute = newTime.minute();
  state.second = newTime.second();
}

void DateTimeModule::getState(DateTimeModuleState& out) const {
  out = state;
}

void DateTimeModule::setState(const DateTimeModuleState& newState) {
  // Аппаратная запись времени выполняется setTime() с моделью DateTime;
  // здесь только фиксируется состояние.
  state = newState;
}
