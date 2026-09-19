#include "test_bms_logic.h"

// Статические буферы строк экрана (длина не превышает Display::LINE_BUF).
static char CURRENT_LINE[Display::LINE_BUF];
static char VOLTAGE_LINE[Display::LINE_BUF];

// Вспомогательный буфер значения (число + знак + десятичная + завершитель).
static char NUMBER_BUF[8];

void TestBmsLogic::init() {
  // Инициализация компонентов выполняется базовым классом DeviceLogic (включая bms_reader).
  DeviceLogic::init();
  // Первичная попытка подключиться к БМС: при ошибке связи BmsReader проставляет
  // флаг, а на первую строку экрана выводится "bms disconnect".
  printBms();
}

void TestBmsLogic::loop() {
  // При установленном флаге ошибки update() внутри printBms() является повторной
  // попыткой подключиться к БМС; успех сбрасывает флаг.
  printBms();
  delay(STATE_CHANGE_DELAY);
}

// Читает БМС и выводит показания либо "bms disconnect".
void TestBmsLogic::printBms() {
  // Попытка прочитать БМС (в том числе повторная попытка подключения при обрыве связи).
  bms_reader.update();
  if (bms_reader.isConnectionError()) {
    // Связь с БМС недоступна (батарея разряжена) — "bms disconnect" в первой строке.
    strcpy(CURRENT_LINE, "bms disconnect");
    VOLTAGE_LINE[0] = '\0';
  } else {
    // Ток на первую строку, напряжение на вторую; значения берутся напрямую из
    // структур драйвера (без промежуточной копии под экономию RAM).
    formatCurrentLine(CURRENT_LINE, bms_reader.getTelemetry().packCurrent);
    formatVoltageLine(VOLTAGE_LINE, bms_reader.getTelemetry().packVoltage);
  }
  display.print(CURRENT_LINE, VOLTAGE_LINE);
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
