#include "test_bms_logic.h"

// Статические буферы строк экрана (длина не превышает Display::LINE_BUF).
static char CURRENT_LINE[Display::LINE_BUF];
static char VOLTAGE_LINE[Display::LINE_BUF];

// Вспомогательный буфер значения (число + знак + десятичная + завершитель).
static char NUMBER_BUF[8];

void TestBmsLogic::init() {
  // Инициализация компонентов выполняется базовым классом DeviceLogic (включая bms_reader).
  DeviceLogic::init();
}

void TestBmsLogic::loop() {
  // Считываем БМС: ток на первую строку, напряжение на вторую.
  if (bms_reader.readBmsValues(readings)) {
    formatCurrentLine(CURRENT_LINE, readings.packCurrent);
    formatVoltageLine(VOLTAGE_LINE, readings.packVoltage);
  } else {
    // БМС не ответила (нет связи) — выводим заглушку.
    strcpy(CURRENT_LINE, "NO BMS");
    strcpy(VOLTAGE_LINE, "DATA LOST");
  }
  display.print(CURRENT_LINE, VOLTAGE_LINE);
  delay(STATE_CHANGE_DELAY);
}

void TestBmsLogic::formatCurrentLine(char* line, float current) {
  // Строка вида "I=-12.3A": до 4 цифр до десятичной и знак.
  memcpy(line, "I=", 2u);
  dtostrf(current, 4, 1u, NUMBER_BUF);
  strcpy(&line[2u], NUMBER_BUF);
  strcat(line, "A");
}

void TestBmsLogic::formatVoltageLine(char* line, float voltage) {
  // Строка вида "U=14.60V": до 3 цифр до десятичной.
  memcpy(line, "U=", 2u);
  dtostrf(voltage, 3, 2u, NUMBER_BUF);
  strcpy(&line[2u], NUMBER_BUF);
  strcat(line, "V");
}

void TestBmsLogic::updateState() {
  // Тестовый вариант не реагирует на смену тарифного периода.
}
