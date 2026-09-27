#pragma once

#include "device_logic.h"
#include "temperature_control.h"

// Тестовый вариант логики устройства: инициализирует TemperatureControl по
// четырём IR-датчикам, опрашивает температуры и выводит статус на экран:
// первая строка — "temperature OK"/"OVERHEETING!", вторая — показания датчиков.
class TestTemperatureControl : public DeviceLogic {
public:
    // Конструктор: показаний ещё нет.
    TestTemperatureControl();

    // Инициализация компонентов устройства и контроллера температуры.
    void init() override;

    // Главный цикл: опрос датчиков и вывод статуса на экран.
    void loop() override;

    // Опрашивает все датчики контроллером температуры и запоминает состояние.
    void read();

    // Перегрузка: дополнительно записывает все текущие показания датчиков
    // (NUM_SENSORS значений, °C) в выходной буфер.
    void read(float* outTemperatures);

protected:
    // Тестовый вариант не реагирует на смену тарифного периода.
    void updateState() override;

private:
    // Формирует строку статуса для первой строки экрана:
    // "temperature OK" — все показания в пределах порогов, иначе "OVERHEETING!".
    void formatStatusLine(char* line);

    // Формирует строку первых трёх показаний для второй строки экрана:
    // значения с одной дробной цифрой через пробел, при отсутствии данных "--.-".
    void formatTemperaturesLine(char* line);

    // Контроллер температуры по четырём IR-датчикам.
    TemperatureControl temperature_control;

    // Последнее состояние контроллера температуры (показания и флаги порогов).
    TemperatureControlState tc_state;
};
