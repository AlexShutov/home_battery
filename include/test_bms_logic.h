#pragma once

#include "bms_reader.h"
#include "device_logic.h"

// Тестовый вариант логики устройства: периодическое считывание телеметрии Daly BMS
// и вывод тока (первая строка) и напряжения (вторая строка) на экран.
class TestBmsLogic : public DeviceLogic {
public:
    // Инициализация компонентов устройства; реализация в базовом классе DeviceLogic.
    void init() override;

    // Главный цикл тестовой логики: опрос БМС и вывод показаний на экран.
    void loop() override;

protected:
    // Тестовый вариант не реагирует на смену тарифного периода.
    void updateState() override;

private:
    // Формирует строку вида "I=-12.3A" (ток, А, знак и одна десятичная).
    void formatCurrentLine(char* line, float current);

    // Формирует строку вида "U=54.10V" (напряжение, В, две десятичные).
    void formatVoltageLine(char* line, float voltage);

    // Последняя телеметрия БМС: выходной буфер для BmsReader::readBmsValues().
    BmsReadings readings;
};
