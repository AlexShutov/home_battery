#pragma once

#include <Arduino.h>

// У заголовка Adafruit_MLX90614.h нет собственного стража включения (и правка в
// .pio/libdeps живёт до переустановки пакета) — оборачиваем включение своим
// макросом. Имя выбрано отличным от возможного библиотечного ADAFRUIT_MLX90614_H.
#ifndef PROJECT_MLX90614_INCLUDED
#define PROJECT_MLX90614_INCLUDED
#include <Adafruit_MLX90614.h>
#endif

// Состояние одного IR-датчика температуры MLX90614.
struct IrSensorState {
    // I2C-адрес датчика.
    uint8_t address;

    // Последнее прочитанное показание, °C (NAN — данных нет).
    float temperature;

    // Успешно ли подключение к датчику.
    bool connected;

    // Сравнивает два состояния на равенство.
    bool operator==(const IrSensorState& other) const {
        return address == other.address && temperature == other.temperature && connected == other.connected;
    }
};

// Один IR-датчик температуры MLX90614: подключение по адресу и чтение показаний.
class IrSensor {
public:
    // Конструктор: датчик ещё не подключен, показаний нет.
    IrSensor();

    // Подключается к датчику по заданному адресу; контрольное чтение отсеивает
    // посторонние I2C-устройства (их показание — NAN). Возвращает true при успехе.
    bool init(uint8_t address);

    // Считывает температуру объекта, °C; при ошибке чтения возвращает NAN.
    float read();

    // Возвращает состояние датчика в выходной параметр.
    void getState(IrSensorState& out) const;

    // Фиксирует состояние; аппаратной части у датчика нет (он только читается).
    void setState(const IrSensorState& newState);

private:
    // Датчик MLX90614.
    Adafruit_MLX90614 sensor;

    // Состояние датчика.
    IrSensorState state;
};
