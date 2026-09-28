#include "set_date_time.h"
#include <Wire.h>
#include <ctype.h>
#include <stdio.h>

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

// Приёмная строка из COM-порта (с запасом под формат даты/времени).
static char RX_LINE[40];

// Текущая позиция записи в приёмную строку.
static uint8_t RX_POS = 0;

// Названия месяцев для вывода: 4 или 3 символа (слот из 5 с завершающим '\0').
static const char MONTH_NAMES[12][5] = {
    "JAN", "FEB", "MARC", "APR", "MAY", "JUNE",
    "JULY", "AUG", "SEPT", "OCT", "NOV", "DEC",
};

// Сравнивает первые три буквы названия месяца без учёта регистра.
static bool matchMonth(const char* str, uint8_t monthIndex) {
  for (uint8_t i = 0; i < 3u; ++i) {
    if (tolower((int)str[i]) != tolower((int)MONTH_NAMES[monthIndex][i])) {
      return false;
    }
  }
  return true;
}

// Разбирает строку вида "datetime: 27 SEPT 2026, 14 : 04" (пробелы вокруг
// запятой и двоеточия не обязательны). Возвращает true при распознавании.
static bool parseDateTimeLine(const char* line, uint8_t& month, uint8_t& day,
                              uint16_t& year, uint8_t& hour, uint8_t& minute) {
  int dayRead = 0;
  int yearRead = 0;
  int hourRead = 0;
  int minuteRead = 0;
  char monthStr[6] = {0};

  if (sscanf(line, "datetime: %d %5s %d, %d : %d", &dayRead, monthStr, &yearRead,
             &hourRead, &minuteRead) != 5) {
    return false;
  }

  // Месяц узнаём по первым трём буквам ("SEP" и "SEPT" — сентябрь).
  uint8_t monthRead = 0;
  bool monthFound = false;
  for (uint8_t m = 0; m < 12u; ++m) {
    if (matchMonth(monthStr, m)) {
      monthRead = (uint8_t)(m + 1u);
      monthFound = true;
      break;
    }
  }
  if (!monthFound) {
    return false;
  }

  // Проверка диапазонов полей.
  if (dayRead < 1 || dayRead > 31 || hourRead < 0 || hourRead > 23 ||
      minuteRead < 0 || minuteRead > 59 || yearRead < 2000 || yearRead > 9999) {
    return false;
  }

  month = monthRead;
  day = (uint8_t)dayRead;
  year = (uint16_t)yearRead;
  hour = (uint8_t)hourRead;
  minute = (uint8_t)minuteRead;
  return true;
}

// Дописывает число (0..99) двумя десятичными символами в строку;
// возвращает новую позицию за дописанным.
static uint8_t appendTwoDigits(char* line, uint8_t pos, uint8_t value) {
  char tens = '0';
  while (value >= 10u) {
    value -= 10u;
    ++tens;
  }
  line[pos] = tens;
  line[pos + 1u] = (char)('0' + value);
  return (uint8_t)(pos + 2u);
}

// Дописывает число (0..9999) четырьмя десятичными символами в строку;
// возвращает новую позицию за дописанным.
static uint8_t appendFourDigits(char* line, uint8_t pos, uint16_t value) {
  char thousands = '0';
  while (value >= 1000u) {
    value -= 1000u;
    ++thousands;
  }

  char hundreds = '0';
  while (value >= 100u) {
    value -= 100u;
    ++hundreds;
  }

  char tens = '0';
  while (value >= 10u) {
    value -= 10u;
    ++tens;
  }

  line[pos] = thousands;
  line[pos + 1u] = hundreds;
  line[pos + 2u] = tens;
  line[pos + 3u] = (char)('0' + value);
  return (uint8_t)(pos + 4u);
}

// Полубайт в hex-символ ('0'..'F').
static char nibbleToHexChar(uint8_t nibble) {
  return (char)(nibble < 10u ? '0' + nibble : 'A' + nibble - 10u);
}

// Сканирует шину I2C и записывает адреса ответивших устройств в строку
// (hex без "0x", через пробел; влезающие в ширину экрана). Возвращает true,
// если на шине найден DS3231 (адрес 0x68).
static bool scanI2CBus(char* line) {
  uint8_t pos = 0;
  bool rtcSeen = false;
  for (uint8_t addr = 1u; addr <= 127u; ++addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() != 0u) {
      continue;
    }
    if (addr == 0x68u) {
      rtcSeen = true;
    }
    // Разделитель-пробел между адресами.
    if (pos != 0u && pos < (uint8_t)(Display::LINE_BUF - 4u)) {
      line[pos] = ' ';
      ++pos;
    }
    if (pos >= (uint8_t)(Display::LINE_BUF - 3u)) {
      continue;
    }
    line[pos] = nibbleToHexChar((uint8_t)(addr >> 4));
    ++pos;
    line[pos] = nibbleToHexChar((uint8_t)(addr & 0x0Fu));
    ++pos;
  }
  line[pos] = '\0';
  return rtcSeen;
}

// Ножка проверки живости модуля: выход 32K DS3231 (32768 Гц) подключается
// на D8 по желанию; без подключения тест неинформативен.
static const uint8_t PIN_32K_TEST = 8;

// Проверяет сигнал 32768 Гц на ножке 32K (если она соединена с D8):
// неподключённая ножка держится подтяжкой в HIGH, а меандр даёт ~50% LOW.
static bool is32kAlive() {
  pinMode(PIN_32K_TEST, INPUT_PULLUP);
  uint16_t lowSamples = 0;
  uint16_t total = 20000u;
  for (uint16_t i = 0; i < total; ++i) {
    if (digitalRead(PIN_32K_TEST) == LOW) {
      ++lowSamples;
    }
  }
  return lowSamples > (uint16_t)(total / 4u);
}

SetDateTime::SetDateTime() {
  dt_state.year = 2000;
  dt_state.month = 1;
  dt_state.day = 1;
  dt_state.hour = 0;
  dt_state.minute = 0;
  dt_state.second = 0;
  dt_state.connected = false;
  dt_state.timeValid = false;
  time_set = false;
}

void SetDateTime::init() {
  // Базовая инициализация компонентов устройства (Serial уже открыт на 9600
  // инициализацией BmsReader).
  DeviceLogic::init();

  // Колбэки фиксируют результат инициализации модуля времени.
  date_time_module.init(onRtcInitOk, onRtcInitFailed);

  // Если время в модуле сохранилось с прошлого включения (живая резервная
  // батарея), сразу показываем его; иначе ждём строку из COM-порта.
  date_time_module.getState(dt_state);
  time_set = RTC_INIT_OK && dt_state.timeValid;
}

void SetDateTime::loop() {
  // Слушаем COM-порт и копим строку до перевода строки.
  while (Serial.available() > 0) {
    char symbol = (char)Serial.read();
    if (symbol == '\r') {
      continue;
    }
    if (symbol == '\n') {
      RX_LINE[RX_POS] = '\0';
      RX_POS = 0;
      processSerialLine();
    } else if (RX_POS < (uint8_t)(sizeof(RX_LINE) - 1u)) {
      RX_LINE[RX_POS] = symbol;
      ++RX_POS;
    } else {
      // Переполнение приёмной строки — начинаем копить заново.
      RX_POS = 0;
    }
  }

  printScreen();
  // Период вызова задаёт задача-планировщик FreeRTOS (500 мс); собственная
  // задержка не нужна — блокировать idle-контекст нельзя.
}

void SetDateTime::processSerialLine() {
  uint8_t month = 1;
  uint8_t day = 1;
  uint16_t year = 2000;
  uint8_t hour = 0;
  uint8_t minute = 0;

  // Чужие строки молча пропускаем — ждём именно строку установки времени.
  if (!parseDateTimeLine(RX_LINE, month, day, year, hour, minute)) {
    return;
  }
  if (!RTC_INIT_OK) {
    // Модуль не отвечает — записывать время некуда.
    return;
  }

  // Секунды в строке не передаются — время стартует с нуля секунд.
  date_time_module.setTime(DateTime(year, month, day, hour, minute, 0));
  time_set = true;
}

void SetDateTime::printScreen() {
  if (!RTC_INIT_OK) {
    // Диагностика: вторая строка — адреса устройств, ответивших на скане шины
    // (0x27 — дисплей, 0x68 — DS3231, 0x57 — EEPROM модуля RTC). Первая строка —
    // тест живости генератора по ножке 32K, если она соединена с D8:
    // "32K ok, no i2c" — модуль жив, но данных линий нет (провода/пайка SDA/SCL);
    // "no 32k signal" — сигнала нет: либо 32K не подключена к D8, либо модуль мёртв.
    bool rtcSeen = scanI2CBus(TIME_LINE);
    strcpy(DATE_LINE, is32kAlive() ? "32K ok, no i2c" : "no 32k signal");
    display.print(DATE_LINE, TIME_LINE);

    // Модуль подключили при работающем устройстве — повторяем инициализацию
    // без перезагрузки.
    if (rtcSeen) {
      date_time_module.init(onRtcInitOk, onRtcInitFailed);
    }
    return;
  }

  if (!time_set) {
    // Ждём строку с временем через COM-порт.
    display.print("waiting time", "send datetime");
    return;
  }

  // Читаем установленное время и показываем его.
  date_time_module.getTime(READ_TIME);
  date_time_module.getState(dt_state);

  // Первая строка — дата вида "27 SEPT 2026".
  uint8_t pos = appendTwoDigits(DATE_LINE, 0u, dt_state.day);
  DATE_LINE[pos] = ' ';
  ++pos;
  uint8_t monthIndex = (dt_state.month >= 1u && dt_state.month <= 12u)
                           ? (uint8_t)(dt_state.month - 1u)
                           : 0u;
  pos += strlen(strcpy(&DATE_LINE[pos], MONTH_NAMES[monthIndex]));
  DATE_LINE[pos] = ' ';
  ++pos;
  pos = appendFourDigits(DATE_LINE, pos, dt_state.year);
  DATE_LINE[pos] = '\0';

  // Вторая строка — время вида "14:04:00".
  pos = appendTwoDigits(TIME_LINE, 0u, dt_state.hour);
  TIME_LINE[pos] = ':';
  ++pos;
  pos = appendTwoDigits(TIME_LINE, pos, dt_state.minute);
  TIME_LINE[pos] = ':';
  ++pos;
  pos = appendTwoDigits(TIME_LINE, pos, dt_state.second);
  TIME_LINE[pos] = '\0';

  display.print(DATE_LINE, TIME_LINE);
}

void SetDateTime::updateState() {
  // Установка времени не привязана к смене тарифного периода.
}
