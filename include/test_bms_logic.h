#pragma once

#include "bms_reader.h"
#include "device_logic.h"

// Тестовый вариант логики устройства: периодическое считывание телеметрии Daly BMS
// и вывод тока (первая строка) и напряжения (вторая строка) на экран. Если связь с БМС
// потеряна (батарея разряжена), выводит "bms disconnect" и каждый цикл повторяет попытку
// подключиться.
class TestBmsLogic : public DeviceLogic {
public:
    // Инициализация компонентов устройства; при ошибке первичного подключения к БМС
    // выводит на первую строку экрана "bms disconnect".
    void init() override;

    // Главный цикл: при установленном флаге ошибки BmsReader повторяет попытку
    // подключиться к БМС и выводит показания (или "bms disconnect").
    void loop() override;

protected:
    // Тестовый вариант не реагирует на смену тарифного периода.
    void updateState() override;

private:
    // Читает БМС (повторяя попытку подключения) и выводит на экран: показания, либо
    // "bms disconnect" в первой строке при ошибке связи.
    void printBms();

    // Формирует строку вида "I=-12.3A" (ток, А, знак и одна десятичная).
    void formatCurrentLine(char* line, float current);

    // Формирует строку вида "U=54.10V" (напряжение, В, две десятичные).
    void formatVoltageLine(char* line, float voltage);
};
