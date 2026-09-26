#include "test_temperature_control.h"
#include <math.h>

// Статическая строка статуса (длина не более Display::LINE_BUF).
static char STATUS_LINE[Display::LINE_BUF];

// Статическая строка показаний датчиков (длина не более Display::LINE_BUF).
static char TEMPERATURES_LINE[Display::LINE_BUF];

// Вспомогательный буфер одного числа (знак + 3 цифры + точка + дробная + '\0').
static char NUMBER_BUF[8];

// Статический буфер всех текущих показаний датчиков для loop().
static float TEMPERATURES_BUF[TemperatureControl::NUM_SENSORS];

// Зафиксированное колбэком событие перегрева (все датчики выше максимума).
static bool EVENT_HOT = false;

// Зафиксированное колбэком событие полного остывания (все датчики ниже минимума).
static bool EVENT_COLD = false;

// Колбэк перегрева: фиксирует событие для вывода текста на экран.
static void onTempTooHigh() {
  EVENT_HOT = true;
  EVENT_COLD = false;
}

// Колбэк полного остывания: фиксирует событие для вывода текста на экран.
static void onCooledDown() {
  EVENT_COLD = true;
  EVENT_HOT = false;
}

TestTemperatureControl::TestTemperatureControl() {
  for (uint8_t i = 0; i < TemperatureControl::NUM_SENSORS; ++i) {
    tc_state.temperatures[i] = NAN;
    tc_state.connected[i] = false;
    tc_state.outOfRange[i] = false;
  }
  tc_state.allCooledDown = false;
  tc_state.allTooHot = false;
}

void TestTemperatureControl::init() {
  // Базовая инициализация компонентов устройства.
  DeviceLogic::init();

  // Колбэки остывания/перегрева фиксируют события для вывода текста на экран.
  // Режим игнорирования неработающих датчиков: их показания не учитываются при
  // агрегировании, ошибки датчиков отображаются по состоянию (connected/NAN).
  // Колбэк ошибки инициализации в этом режиме контроллером не вызывается.
  temperature_control.init(onCooledDown, onTempTooHigh, nullptr, true);
}

void TestTemperatureControl::loop() {
  // Опрос всех датчиков с записью показаний в буфер.
  read(TEMPERATURES_BUF);

  // Первая строка — статус, вторая — показания первых трёх датчиков.
  formatStatusLine(STATUS_LINE);
  formatTemperaturesLine(TEMPERATURES_LINE);
  display.print(STATUS_LINE, TEMPERATURES_LINE);

  delay(STATE_CHANGE_DELAY);
}

void TestTemperatureControl::read() {
  // Опрос всех датчиков и обновление флагов порогов внутри контроллера.
  temperature_control.loop();
  // Состояние читаем только через getState().
  temperature_control.getState(tc_state);
}

void TestTemperatureControl::read(float* outTemperatures) {
  // Опрос датчиков, затем копирование всех текущих показаний в буфер.
  read();
  for (uint8_t i = 0; i < TemperatureControl::NUM_SENSORS; ++i) {
    outTemperatures[i] = tc_state.temperatures[i];
  }
}

// Формирует строку статуса: ошибка датчика, перегрев, остывание или норма.
void TestTemperatureControl::formatStatusLine(char* line) {
  // Нездоровый датчик (не инициализировался или потерян в работе) — приоритетнее
  // событий температуры: сбой связи — это не «перегрев».
  for (uint8_t i = 0; i < TemperatureControl::NUM_SENSORS; ++i) {
    if (!tc_state.connected[i] || isnan(tc_state.temperatures[i])) {
      strcpy(line, "SENSOR ");
      line[7] = (char)('0' + i + 1u);
      strcpy(&line[8], " ERROR");
      return;
    }
  }

  // Живая проверка направления выхода за пороги по ответившим датчикам.
  bool anyAboveMax = false;
  bool allBelowMin = true;
  for (uint8_t i = 0; i < TemperatureControl::NUM_SENSORS; ++i) {
    if (tc_state.temperatures[i] > (float)TemperatureControl::MAX_THRESHOLD_TEMPS[i]) {
      anyAboveMax = true;
    }
    if (!(tc_state.temperatures[i] < (float)TemperatureControl::MIN_THRESHOLD_TEMPS[i])) {
      allBelowMin = false;
    }
  }

  // События колбэков показываем, пока условие держится; при нормализации — сброс.
  EVENT_HOT = anyAboveMax;
  EVENT_COLD = allBelowMin && !anyAboveMax;

  if (EVENT_HOT) {
    strcpy(line, "OVERHEETING!");
  } else if (EVENT_COLD) {
    strcpy(line, "COOLED DOWN");
  } else {
    strcpy(line, "temperature OK");
  }
}

// Формирует строку показаний первых трёх датчиков через пробел.
void TestTemperatureControl::formatTemperaturesLine(char* line) {
  const uint8_t TEMPERATURES_ON_SCREEN = 3;
  uint8_t pos = 0;
  for (uint8_t i = 0; i < TEMPERATURES_ON_SCREEN; ++i) {
    if (i != 0u) {
      line[pos] = ' ';
      ++pos;
    }
    if (isnan(tc_state.temperatures[i])) {
      // Датчик не отвечает — заглушка той же ширины.
      memcpy(&line[pos], "--.-", 4u);
      pos += 4u;
    } else {
      dtostrf(tc_state.temperatures[i], 0, 1u, NUMBER_BUF);
      strcpy(&line[pos], NUMBER_BUF);
      pos += strlen(NUMBER_BUF);
    }
  }
  line[pos] = '\0';
}

void TestTemperatureControl::updateState() {
  // Тестовый вариант не реагирует на смену тарифного периода.
}
