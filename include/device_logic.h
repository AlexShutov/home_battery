#pragma once

#include <Arduino.h>
#include "bms_reader.h"
#include "display.h"
#include "device_screen.h"
#include "relays.h"
#include "time_switcher.h"

// Базовый абстрактный класс логики устройства: хранит компоненты и состояние,
// реализует общую инициализацию и опрос состояния.
class DeviceLogic {
protected:
    // Инициализирует ножки и компоненты устройства, считывает начальное состояние и выводит его на дисплей.
    virtual void init();

    // Главный цикл логики устройства; реализация обязана быть в производном классе.
    virtual void loop() = 0;

    // Реакция на изменение тарифного периода; реализация обязана быть в производном классе.
    virtual void updateState() = 0;

    // Считывает актуальное состояние из TimeSwitcher и Relays в выходной параметр.
    void readDeviceState(DeviceState& state);

    // Считывает актуальное состояние; при изменении тарифного периода вызывает updateState().
    void update();

    // Пауза между включением и выключением реле, мс.
    static const uint16_t RELAY_TIME = 500;

    // Задержка после изменения состояния устройства, мс.
    static const uint16_t STATE_CHANGE_DELAY = 1000;

    // Дисплей устройства.
    Display display;

    // Реле устройства.
    Relays relays;

    // Чтец телеметрии Daly BMS (Serial, 9600 8N1).
    BmsReader bms_reader;

    // Экран с выводом состояния устройства.
    DeviceScreen screen;

    // Выключатель тарифных периодов (ножки D6/D7).
    TimeSwitcher time_switcher;

    // Последнее зафиксированное состояние устройства.
    DeviceState device_state;

    // Новое прочитанное состояние устройства.
    DeviceState new_device_state;
};
