#pragma once

#include "device_logic.h"
#include "date_time_module.h"

// Тестовый вариант логики устройства: инициализирует модуль времени DS3231
// и выводит на экран дату ("27 SEPT 2026") и время ("HH:MM:SS"). При неудачной
// инициализации — "date time" на первой строке и "unavailable" на второй.
class DateTimeTest : public DeviceLogic {
public:
    // Конструктор: модуль ещё не инициализирован, время обнулено.
    DateTimeTest();

    // Инициализация компонентов устройства и модуля времени.
    void init() override;

    // Главный цикл: чтение времени и вывод на экран.
    void loop() override;

protected:
    // Тестовый вариант не реагирует на смену тарифного периода.
    void updateState() override;

private:
    // Формирует строку даты "DD MMM YYYY" для первой строки экрана;
    // название месяца — 4 символа (может быть из трёх, например "JAN").
    void formatDateLine(char* line);

    // Формирует строку времени "HH:MM:SS" для второй строки экрана.
    void formatTimeLine(char* line);

    // Модуль реального времени DS3231.
    DateTimeModule date_time_module;

    // Последнее состояние модуля времени.
    DateTimeModuleState dt_state;
};
