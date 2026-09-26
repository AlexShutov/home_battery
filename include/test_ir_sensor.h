#pragma once

#include "device_logic.h"
// У заголовка Adafruit_MLX90614.h нет собственного стража включения (и правка в
// .pio/libdeps живёт до переустановки пакета) — оборачиваем включение своим
// макросом. Имя выбрано отличным от возможного библиотечного ADAFRUIT_MLX90614_H.
#ifndef PROJECT_MLX90614_INCLUDED
#define PROJECT_MLX90614_INCLUDED
#include <Adafruit_MLX90614.h>
#endif

// Тестовый вариант логики устройства: сканирование шины I2C в поисках датчиков MLX90614
// и вывод их показаний температуры на строки экрана.
class TestIRSensor : public DeviceLogic {
public:
    // Конструктор: обнуляет счётчик найденных датчиков.
    TestIRSensor();

    // Инициализация компонентов устройства и поиск датчиков на шине I2C.
    void init() override;

    // Считывает температуру с найденных датчиков и выводит её на строки экрана.
    void loop() override;

protected:
    // Тестовый вариант не реагирует на смену тарифного периода.
    void updateState() override;

private:
    // Максимальное количество датчиков, выводимых на экран (по одному на строку).
    static const uint8_t MAX_SENSORS = 2;

    // Сканирует шину I2C и инициализирует до MAX_SENSORS найденных датчиков.
    void findSensors();

    // Формирует строку с показанием датчика по индексу (0 — первая строка, 1 — вторая).
    void formatLine(char* line, uint8_t index);

    // Найденные и инициализированные датчики MLX90614.
    Adafruit_MLX90614 sensors[MAX_SENSORS];

    // Количество найденных датчиков (не больше MAX_SENSORS).
    uint8_t sensor_count;
};
