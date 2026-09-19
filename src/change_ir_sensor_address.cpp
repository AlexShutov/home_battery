#include "change_ir_sensor_address.h"
#include <math.h>

const uint8_t ChangeIRSensorAddress::DEFAULT_IR_ADDR;
const char* const ChangeIRSensorAddress::NEW_IR_ADDR = "0x5B";

// Адрес дисплея на шине I2C (см. Display) — исключается при поиске датчика.
static const uint8_t DISPLAY_I2C_ADDRESS = 0x27;

// Регистр EEPROM MLX90614, хранящий SMBus-адрес датчика.
static const uint8_t MLX90614_ADDR_REG = 0x2E;

// Строки результата по индексу: 0 — успех, 1 — ошибка.
static const char* const ADDR_RESULT[2] = { "addr chng ok", "adr chn er" };

// Статический буфер первой строки экрана (длина не более Display::LINE_BUF).
static char ADDR_LINE[Display::LINE_BUF];

// Ищет датчик MLX90614: сначала стандартный (дефолтный) адрес, затем сканирование
// остальных адресов; 0 — устройств не найдено.
static uint8_t scanI2CAddress(uint8_t defaultAddr) {
  // Приоритет — датчик на стандартном адресе, его адрес и меняем.
  Wire.beginTransmission(defaultAddr);
  if (Wire.endTransmission() == 0u) {
    return defaultAddr;
  }
  for (uint8_t addr = 1u; addr <= 127u; ++addr) {
    if (addr == DISPLAY_I2C_ADDRESS || addr == defaultAddr) {
      continue;
    }
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0u) {
      return addr;
    }
  }
  return 0u;
}

// Разбирает константную строку адреса вида "0x5B" в число.
static uint8_t parseAddress(const char* addrStr) {
  return (uint8_t)strtol(addrStr, nullptr, 16);
}

// Один шаг CRC-8 (PEC) пакета SMBus — как в библиотеке Adafruit_MLX90614.
static uint8_t smbusCrcStep(uint8_t crc, uint8_t byte) {
  for (uint8_t i = 8u; i != 0u; --i) {
    uint8_t carry = (uint8_t)((crc ^ byte) & 0x80u);
    crc <<= 1;
    if (carry != 0u) {
      crc ^= 0x07u;
    }
    byte <<= 1;
  }
  return crc;
}

// Записывает 16-битное слово в EEPROM MLX90614 (с PEC); true — запись принята.
static bool writeEepromWord(uint8_t deviceAddr, uint8_t reg, uint16_t value) {
  uint8_t pec = 0;
  pec = smbusCrcStep(pec, (uint8_t)(deviceAddr << 1));
  pec = smbusCrcStep(pec, reg);
  pec = smbusCrcStep(pec, (uint8_t)(value & 0xFFu));
  pec = smbusCrcStep(pec, (uint8_t)(value >> 8));

  Wire.beginTransmission(deviceAddr);
  Wire.write(reg);
  Wire.write((uint8_t)(value & 0xFFu));
  Wire.write((uint8_t)(value >> 8));
  Wire.write(pec);
  return Wire.endTransmission() == 0u;
}

ChangeIRSensorAddress::ChangeIRSensorAddress() {
  current_addr = 0;
  state.isAddressChanged = false;
}

void ChangeIRSensorAddress::init() {
  // Инициализация компонентов выполняется базовым классом DeviceLogic.
  DeviceLogic::init();

  // Смена адреса выполняется один раз при старте (аналог setup()).
  changeAddress();
  printResult();
}

void ChangeIRSensorAddress::loop() {
  // Результат смены адреса остаётся на дисплее.
  delay(STATE_CHANGE_DELAY);
}

bool ChangeIRSensorAddress::writeIrSensorAddress() {
  uint8_t newAddr = parseAddress(NEW_IR_ADDR);

  // Записываем в EEPROM новый адрес (стирание ячейки и повторная запись — как в
  // библиотеке для EEPROM). Адрес хранится в битах [7:1] младшего байта ячейки,
  // поэтому значение сдвигаем на 1. Новый адрес датчик применит после сброса
  // (выключения питания), поэтому успех считаем по принятой записи.
  bool ok = writeEepromWord(current_addr, MLX90614_ADDR_REG, 0x0000);
  delay(10);
  ok = writeEepromWord(current_addr, MLX90614_ADDR_REG, (uint16_t)(newAddr << 1)) && ok;
  delay(10);
  return ok;
}

void ChangeIRSensorAddress::changeAddress() {
  // По умолчанию считаем, что смена адреса не удалась.
  state.isAddressChanged = false;

  // Сканирование всех адресов и подключение к датчику.
  current_addr = scanI2CAddress(DEFAULT_IR_ADDR);
  if (current_addr != 0u && ir_sensor.begin(current_addr)) {
    // Контрольное чтение температуры: при ошибке чтения (NAN) по адресу ответил
    // посторонний I2C-девайс, а не MLX90614 — в EEPROM чужого устройства не пишем.
    if (isnan(ir_sensor.readObjectTempC())) {
      return;
    }
    // Подключение успешно: меняем адрес. Так как датчик применит новый адрес после
    // сброса (выключения питания), флаг фиксируем по успешной записи адреса в EEPROM.
    state.isAddressChanged = writeIrSensorAddress();
  }
}

void ChangeIRSensorAddress::printResult() {
  uint8_t index = state.isAddressChanged ? 0u : 1u;
  strcpy(ADDR_LINE, ADDR_RESULT[index]);
  display.print(ADDR_LINE, "");
}

void ChangeIRSensorAddress::getState(IrSensorAddressState& out) const {
  out = state;
}

void ChangeIRSensorAddress::setState(const IrSensorAddressState& newState) {
  // Аппаратно адрес записывается методом writeIrSensorAddress(); здесь фиксируется
  // только флаг успешности смены адреса.
  state = newState;
}

void ChangeIRSensorAddress::updateState() {
  // Смена адреса не привязана к смене тарифного периода.
}