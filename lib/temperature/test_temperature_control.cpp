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

// Номер датчика, не прошедшего инициализацию (0 — все инициализированы).
// Запоминается первым сбойным, чтобы сообщение на экране было детерминированным.
static uint8_t FAILED_SENSOR_NUM = 0;

// Колбэк ошибки инициализации датчика: фиксирует номер сбойного датчика.
static void onSensorInitFailed(uint8_t sensorNumber) {
  if (FAILED_SENSOR_NUM == 0u) {
    FAILED_SENSOR_NUM = sensorNumber;
  }
}

TestTemperatureControl::TestTemperatureControl() {
  for (uint8_t i = 0; i < TemperatureControl::NUM_SENSORS; ++i) {
    tc_state.temperatures[i] = NAN;
    tc_state.outOfRange[i] = false;
  }
  tc_state.allCooledDown = false;
  tc_state.allTooHot = false;
}

void TestTemperatureControl::init() {
  // Базовая инициализация компонентов устройства.
  DeviceLogic::init();

  // Колбэки остывания/перегрева не нужны: статус выводится на экран самим классом.
  // Режим игнорирования неработающих датчиков: Sensor X ERROR не показываем,
  // статус считается только по ответившим датчикам (например, при нехватке
  // датчиков на стенде).
  temperature_control.init(nullptr, nullptr, onSensorInitFailed, true);
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

// Формирует строку статуса: ошибка датчика, температура в норме или перегрев.
void TestTemperatureControl::formatStatusLine(char* line) {
  // Ошибка инициализации имеет приоритет: сбойный датчик — не «перегрев».
  if (FAILED_SENSOR_NUM != 0u) {
    strcpy(line, "SENSOR ");
    line[7] = (char)('0' + FAILED_SENSOR_NUM);
    strcpy(&line[8], " ERROR");
    return;
  }

  bool ok = true;
  for (uint8_t i = 0; i < TemperatureControl::NUM_SENSORS; ++i) {
    // Неработающий датчик (не инициализировался) в статусе не учитывается.
    if (!tc_state.connected[i]) {
      continue;
    }
    // Выход за пороги (min/max) или потеря связи в процессе работы — не в норме.
    if (tc_state.outOfRange[i] || isnan(tc_state.temperatures[i])) {
      ok = false;
    }
  }
  if (ok) {
    strcpy(line, "temperature OK");
  } else {
    strcpy(line, "OVERHEETING!");
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
