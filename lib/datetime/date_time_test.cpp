#include "date_time_test.h"

// Статическая строка даты (длина не более Display::LINE_BUF).
static char DATE_LINE[Display::LINE_BUF];

// Статическая строка времени (длина не более Display::LINE_BUF).
static char TIME_LINE[Display::LINE_BUF];

// Буфер модели времени библиотеки RTClib для чтения показаний модуля.
static DateTime READ_TIME;

// Результат инициализации модуля времени (фиксируется колбэками).
static bool RTC_INIT_OK = false;

// Колбэк успешной инициализации модуля времени.
static void onRtcInitOk() {
  RTC_INIT_OK = true;
}

// Колбэк неудачной инициализации модуля времени.
static void onRtcInitFailed() {
  RTC_INIT_OK = false;
}

// Названия месяцев: 4 или 3 символа (слот из 5 с завершающим '\0').
static const char MONTH_NAMES[12][5] = {
    "JAN", "FEB", "MARC", "APR", "MAY", "JUNE",
    "JULY", "AUG", "SEPT", "OCT", "NOV", "DEC",
};

// Дописывает число (0..99) двумя десятичными символами в строку;
// возвращает новую позицию за дописанным.
static uint8_t appendTwoDigits(char* line, uint8_t pos, uint8_t value) {
  // Старший символ — десятки: считаем их последовательным вычитанием десятка.
  char tens = '0';
  while (value >= 10u) {
    value -= 10u;
    ++tens;
  }

  // Младший символ — то, что осталось от числа (единицы).
  line[pos] = tens;
  line[pos + 1u] = (char)('0' + value);
  return (uint8_t)(pos + 2u);
}

// Дописывает число (0..9999) четырьмя десятичными символами в строку;
// возвращает новую позицию за дописанным.
static uint8_t appendFourDigits(char* line, uint8_t pos, uint16_t value) {
  // Тысячи: считаем последовательным вычитанием тысячи.
  char thousands = '0';
  while (value >= 1000u) {
    value -= 1000u;
    ++thousands;
  }

  // Сотни: тем же способом.
  char hundreds = '0';
  while (value >= 100u) {
    value -= 100u;
    ++hundreds;
  }

  // Десятки.
  char tens = '0';
  while (value >= 10u) {
    value -= 10u;
    ++tens;
  }

  // Единицы — то, что осталось от числа.
  line[pos] = thousands;
  line[pos + 1u] = hundreds;
  line[pos + 2u] = tens;
  line[pos + 3u] = (char)('0' + value);
  return (uint8_t)(pos + 4u);
}

DateTimeTest::DateTimeTest() {
  dt_state.year = 2000;
  dt_state.month = 1;
  dt_state.day = 1;
  dt_state.hour = 0;
  dt_state.minute = 0;
  dt_state.second = 0;
  dt_state.connected = false;
}

void DateTimeTest::init() {
  // Базовая инициализация компонентов устройства.
  DeviceLogic::init();

  // Колбэки фиксируют результат инициализации; при неудаче loop() выводит
  // "date time"/"unavailable" вместо даты и времени.
  date_time_module.init(onRtcInitOk, onRtcInitFailed);
}

void DateTimeTest::loop() {
  if (RTC_INIT_OK) {
    // Читаем время модуля и его состояние (состояние — только через getState()).
    date_time_module.getTime(READ_TIME);
    date_time_module.getState(dt_state);

    // Первая строка — дата, вторая — время.
    formatDateLine(DATE_LINE);
    formatTimeLine(TIME_LINE);
    display.print(DATE_LINE, TIME_LINE);
  } else {
    // Модуль не инициализировался — время не читаем.
    display.print("date time", "unavailable");
  }

  delay(STATE_CHANGE_DELAY);
}

// Формирует строку даты вида "27 SEPT 2026".
void DateTimeTest::formatDateLine(char* line) {
  // День двумя цифрами.
  uint8_t pos = appendTwoDigits(line, 0u, dt_state.day);
  line[pos] = ' ';
  ++pos;

  // Название месяца (3-4 символа); при повреждённом значении — январь.
  uint8_t monthIndex = (dt_state.month >= 1u && dt_state.month <= 12u)
                           ? (uint8_t)(dt_state.month - 1u)
                           : 0u;
  pos += strlen(strcpy(&line[pos], MONTH_NAMES[monthIndex]));
  line[pos] = ' ';
  ++pos;

  // Год четырьмя цифрами.
  pos = appendFourDigits(line, pos, dt_state.year);
  line[pos] = '\0';
}

// Формирует строку времени вида "12:34:56".
void DateTimeTest::formatTimeLine(char* line) {
  uint8_t pos = appendTwoDigits(line, 0u, dt_state.hour);
  line[pos] = ':';
  ++pos;
  pos = appendTwoDigits(line, pos, dt_state.minute);
  line[pos] = ':';
  ++pos;
  pos = appendTwoDigits(line, pos, dt_state.second);
  line[pos] = '\0';
}

void DateTimeTest::updateState() {
  // Тестовый вариант не реагирует на смену тарифного периода.
}
