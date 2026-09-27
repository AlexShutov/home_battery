#pragma once

#include "device_logic.h"
#include "date_time_module.h"

// Логика установки времени: слушает COM-порт (Serial, через который заливается
// прошивка) и ожидает строку вида "datetime: 27 SEPT 2026, 14 : 04". Распознанное
// время записывается в модуль RTC DS3231; экран показывает установленное время.
class SetDateTime : public DeviceLogic {
public:
    // Конструктор: время ещё не установлено.
    SetDateTime();

    // Инициализация компонентов устройства и модуля времени.
    void init() override;

    // Главный цикл: чтение строк из COM-порта и вывод на экран.
    void loop() override;

protected:
    // Установка времени не привязана к смене тарифного периода.
    void updateState() override;

private:
    // Разбирает накопленную строку; при распознавании пишет время в модуль.
    void processSerialLine();

    // Выводит на экран установленное время (или состояние ожидания).
    void printScreen();

    // Модуль реального времени DS3231.
    DateTimeModule date_time_module;

    // Последнее состояние модуля времени.
    DateTimeModuleState dt_state;

    // Установлено ли время по строке из COM-порта.
    bool time_set;
};
