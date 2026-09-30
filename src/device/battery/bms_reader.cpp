#include "bms_reader.h"

// Привязываем драйвер к единственному аппаратному UART контроллера (Serial).
BmsReader::BmsReader() : bms(Serial), connectionError(false) {}

void BmsReader::init() {
  // Настраивает последовательный порт (9600, 8N1) по спецификации Daly.
  bms.Init();
}

bool BmsReader::update() {
  // Запрашивает у БМС все блоки данных; update() заполняет структуры get/alarm,
  // которые хранятся в единственном экземпляре внутри драйвера.
  bool ok = bms.update();
  // БМС не ответила по UART (считаем, что батарея разряжена и БМС отключилась) —
  // запоминаем ошибку; успешное чтение её сбрасывает.
  connectionError = !ok;
  return ok;
}

bool BmsReader::isConnectionError() const {
  return connectionError;
}

const BmsReader::Telemetry& BmsReader::getTelemetry() const {
  return bms.get;
}

const BmsReader::Alarms& BmsReader::getAlarms() const {
  return bms.alarm;
}
