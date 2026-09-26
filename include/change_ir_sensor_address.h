#pragma once

#include "device_logic.h"
// У заголовка Adafruit_MLX90614.h нет собственного стража включения (и правка в
// .pio/libdeps живёт до переустановки пакета) — оборачиваем включение своим
// макросом. Имя выбрано отличным от возможного библиотечного ADAFRUIT_MLX90614_H.
#ifndef PROJECT_MLX90614_INCLUDED
#define PROJECT_MLX90614_INCLUDED
#include <Adafruit_MLX90614.h>
#endif

// Состояние смены адреса IR-датчика.
struct IrSensorAddressState {
    // Успешно ли изменён адрес датчика на новый.
    bool isAddressChanged;

    // Сравнивает два состояния на равенство.
    bool operator==(const IrSensorAddressState& other) const {
        return isAddressChanged == other.isAddressChanged;
    }
};

// Логика смены стандартного I2C-адреса IR-датчика MLX90614 на новый, заданный
// константной строкой, чтобы на шину можно было подключать несколько датчиков.
class ChangeIRSensorAddress : public DeviceLogic {
public:
    // Конструктор: обнуляет текущий адрес и флаг успешной смены адреса.
    ChangeIRSensorAddress();

    // Инициализация: сканирование шины, смена адреса датчика и вывод результата.
    void init() override;

    // Главный цикл логики смены адреса датчика.
    void loop() override;

    // Записывает в EEPROM датчика новый адрес, заданный константной строкой.
    // Возвращает true, если запись принята датчиком.
    bool writeIrSensorAddress();

    // Возвращает текущее состояние смены адреса в выходной параметр.
    void getState(IrSensorAddressState& out) const;

    // Обновляет состояние смены адреса; аппаратная запись адреса в датчик
    // выполняется методом writeIrSensorAddress().
    void setState(const IrSensorAddressState& newState);

protected:
    // Смена адреса не привязана к смене тарифного периода.
    void updateState() override;

private:
    // Стандартный (заводской) I2C-адрес датчика MLX90614.
    static const uint8_t DEFAULT_IR_ADDR = 0x5A;

    // Новый I2C-адрес датчика, заданный константной строкой.
    static const char* const NEW_IR_ADDR;

    // Сканирует шину, подключается к датчику и меняет его адрес.
    void changeAddress();

    // Выводит результат смены адреса на первую строку дисплея.
    void printResult();

    // Датчик MLX90614, чей адрес меняется.
    Adafruit_MLX90614 ir_sensor;

    // Адрес, по которому датчик обнаружен на шине (нужен для записи в EEPROM).
    uint8_t current_addr;

    // Текущее состояние смены адреса.
    IrSensorAddressState state;
};