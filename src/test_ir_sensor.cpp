#include "test_ir_sensor.h"
#include <Wire.h>

const uint8_t TestIRSensor::MAX_SENSORS;

// Стандартный I2C-адрес дисплея (см. Display) — он не является температурным датчиком.
static const uint8_t DISPLAY_I2C_ADDRESS = 0x27;

// Вспомогательный буфер значения температуры в виде числа с одной дробной цифрой.
static char TEMP_BUF[8];

// Статические буферы строк экрана с показаниями датчиков (длина не превышает Display::LINE_BUF).
static char SENSOR_LINE_1[Display::LINE_BUF];
static char SENSOR_LINE_2[Display::LINE_BUF];

TestIRSensor::TestIRSensor() {
  sensor_count = 0;
}

void TestIRSensor::init() {
  // Базовая инициализация компонентов устройства.
  DeviceLogic::init();

  findSensors();
}

// Сканирует адреса I2C-шины: ответившие устройства, кроме дисплея, считаются датчиками MLX90614.
void TestIRSensor::findSensors() {
  sensor_count = 0;

  for (uint8_t addr = 1u; addr <= 127u && sensor_count < MAX_SENSORS; ++addr) {
    // Дисплей занимает свой адрес — пропустить его при поиске датчиков.
    if (addr == DISPLAY_I2C_ADDRESS) {
      continue;
    }

    Wire.beginTransmission(addr);
    // Опрос завершён с ошибкой — устройства по этому адресу нет.
    if (Wire.endTransmission() != 0u) {
      continue;
    }

    // Устройство нашлось: инициализируем объект датчика для этого адреса.
    sensors[sensor_count].begin(addr);
    ++sensor_count;
  }
}

void TestIRSensor::loop() {
  // Первая строка — показание первого датчика, вторая — второго.
  formatLine(SENSOR_LINE_1, 0u);
  formatLine(SENSOR_LINE_2, 1u);
  display.print(SENSOR_LINE_1, SENSOR_LINE_2);

  delay(STATE_CHANGE_DELAY);
}

// Формирует строку вида "T1=23.5C"; если датчик не найден — выводит заглушку.
void TestIRSensor::formatLine(char* line, uint8_t index) {
  // Подпись строки с номером датчика ("T1" / "T2").
  if (index == 0u) {
    memcpy(line, "T1=", 3u);
  } else {
    memcpy(line, "T2=", 3u);
  }

  if (index >= sensor_count) {
    // Датчик не найден — выводим заглушку вместо показания.
    strcpy(&line[3], "--.-");
    return;
  }

  // Показание с одной дробной цифрой сначала попадает во вспомогательный буфер.
  dtostrf(sensors[index].readObjectTempC(), 0, 1u, TEMP_BUF);
  strcpy(&line[3], TEMP_BUF);

  // Единица измерения на конце строки.
  strcat(line, "C");
}

void TestIRSensor::updateState() {
  // Тестовый вариант не реагирует на смену тарифного периода.
}
