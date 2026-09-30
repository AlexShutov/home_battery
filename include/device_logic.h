#pragma once

#include <Arduino.h>
#include "chargecontrol/charge_control.h"
#include "device/battery/battery_reader.h"
#include "device_screen.h"
#include "display.h"
#include "time_switcher.h"

// Базовый абстрактный класс логики устройства: хранит компоненты и состояние,
// реализует общую инициализацию и опрос состояния.
class DeviceLogic {
public:
    // Конструктор: связывает читатель батареи с читателем телеметрии БМС.
    DeviceLogic();

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

    // Контроль зарядок станции (реле зарядок и их целевое состояние).
    ChargeControl charge_control;

    // Чтец телеметрии Daly BMS (Serial, 9600 8N1) и рассчитанные по ней
    // показания батареи.
    BmsReader bms_reader;
    BatteryReader battery_reader;

    // Экран с выводом состояния устройства.
    DeviceScreen screen;

    // Выключатель тарифных периодов (ножки D6/D7).
    TimeSwitcher time_switcher;

    // Последнее зафиксированное состояние устройства.
    DeviceState device_state;

    // Новое прочитанное состояние устройства.
    DeviceState new_device_state;
};
