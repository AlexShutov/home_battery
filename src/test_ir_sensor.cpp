#include "test_ir_sensor.h"
#include <Wire.h>
#include <math.h>

const uint8_t TestIRSensor::MAX_SENSORS;

// Стандартный I2C-адрес дисплея (см. Display) — он не является температурным датчиком.
static const uint8_t DISPLAY_I2C_ADDRESS = 0x27;

// Вспомогательный буфер значения температуры в виде числа с одной дробной цифрой.
static char TEMP_BUF[8];

// Последнее показание температуры датчика (для проверки NAN).
static float CURRENT_TEMP;

// Полубайт в hex-символ ('0'..'F') для вывода адреса датчика.
static char nibbleToHexChar(uint8_t nibble) {
  return (char)(nibble < 10u ? '0' + nibble : 'A' + nibble - 10u);
}

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

    // Устройство нашлось: инициализируем объект датчика для этого адреса. Ошибка
    // инициализации означает, что по адресу не настоящий MLX90614 — пропускаем.
    if (!sensors[sensor_count].begin(addr)) {
      continue;
    }
    // Контрольное чтение: стараемся не принять за датчик посторонний I2C-девайс
    // (его чтение температуры даёт NAN).
    if (isnan(sensors[sensor_count].readObjectTempC())) {
      continue;
    }
    // Запоминаем адрес найденного датчика — он выводится на экран после температуры.
    sensor_addrs[sensor_count] = addr;
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
  CURRENT_TEMP = sensors[index].readObjectTempC();
  // Чтение не удалось (датчик отключился или адрес не MLX90614) — выводим заглушку.
  if (isnan(CURRENT_TEMP)) {
    strcpy(&line[3], "--.-");
    return;
  }
  dtostrf(CURRENT_TEMP, 0, 1u, TEMP_BUF);
  strcpy(&line[3], TEMP_BUF);

  // Единица измерения на конце строки.
  strcat(line, "C");

  // После температуры через пробел — адрес датчика в hex (строка вида "T1=23.5C 0x5B").
  strcat(line, " 0x");
  uint8_t addr = sensor_addrs[index];
  TEMP_BUF[0] = nibbleToHexChar((uint8_t)(addr >> 4));
  TEMP_BUF[1] = nibbleToHexChar((uint8_t)(addr & 0x0Fu));
  TEMP_BUF[2] = '\0';
  strcat(line, TEMP_BUF);
}

void TestIRSensor::updateState() {
  // Тестовый вариант не реагирует на смену тарифного периода.
}
