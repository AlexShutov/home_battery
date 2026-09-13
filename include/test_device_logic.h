#pragma once

#include "device_logic.h"

// Тестовый вариант логики устройства: периодический опрос состояния и реакция на смену тарифного периода.
class TestDeviceLogic : public DeviceLogic {
public:
    // Инициализация компонентов устройства; реализация в базовом классе DeviceLogic.
    void init() override;

    // Главный цикл тестовой логики устройства.
    void loop() override;

protected:
    // Реакция на смену тарифного периода: кратковременный цикл всех реле.
    void updateState() override;

private:
    // Кратковременно включает и выключает одно реле с паузой RELAY_TIME.
    void turnRelay(uint8_t relay);
};
