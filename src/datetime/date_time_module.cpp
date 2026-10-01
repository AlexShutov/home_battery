#include "date_time_module.h"

DateTimeModule::DateTimeModule() {
  state.year = 2000;
  state.month = 1;
  state.day = 1;
  state.hour = 0;
  state.minute = 0;
  state.second = 0;
  state.connected = false;
  state.timeValid = false;
}

bool DateTimeModule::init(DateTimeCallback onInitOk, DateTimeCallback onInitFailed) {
  // begin() ищет DS3231 на шине I2C (перед этим Wire уже инициализирован шиной
  // дисплея в DeviceLogic::init). Критерий успеха — отклик чипа.
  state.connected = rtc.begin();

  // lostPower поднят, если при включении не было резервного питания и время
  // сброшено (нет/села батарея, первое включение). Запоминаем валидность ДО
  // корректировки: adjust() может не сбросить флаг, а смысл — «время сохранилось
  // с прошлого включения».
  state.timeValid = state.connected && !rtc.lostPower();

  // При сброшенном времени стартуем с даты/времени компиляции прошивки —
  // стандартный приём из примеров RTClib. Точное время установит потребитель.
  if (state.connected && !state.timeValid) {
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }

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

// Проверяет правдоподобность прочитанного времени. RTClib ничего не сообщает
// об ошибке связи, но если DS3231 отсутствует на шине, now() возвращает модель
// из неинициализированных регистров — почти наверняка вне допустимых диапазонов.
static bool isPlausibleTime(const DateTime& t) {
  return t.year() >= 2000u && t.year() <= 2099u && t.month() >= 1u &&
         t.month() <= 12u && t.day() >= 1u && t.day() <= 31u && t.hour() <= 23u &&
         t.minute() <= 59u && t.second() <= 59u;
}

void DateTimeModule::getTime(DateTime& out) {
  // now() возвращает модель библиотеки по значению — переносим её в выходной
  // параметр (модель передаётся по ссылке, локальные объекты не создаются).
  out = rtc.now();

  // Обновляем флаги подключения и валидности по результату чтения, чтобы
  // состояние не оставалось устаревшим (модуль мог пропасть с шины или время
  // могло сброситься после инициализации). Валидность считаем только по
  // правдоподобности показания — факт отклика чипа уже учтён в connected.
  state.connected = isPlausibleTime(out);
  state.timeValid = state.connected;

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

  // После записи время заведомо установлено и валидно.
  state.connected = true;
  state.timeValid = true;

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
