#pragma once

#include <stdint.h>
#include <tinyfsm.hpp>

#include "time_switcher.h"

// Режим зарядки, соответствующий текущему состоянию машины.
enum ChargingMode : uint8_t {
    // Зарядка выключена.
    CHARGING_MODE_OFF = 0,

    // Зарядка включена на минимальную мощность.
    CHARGING_MODE_MINIMAL,

    // Зарядка включена на максимальную мощность.
    CHARGING_MODE_MAX,

    // Зарядка остановлена из-за перегрева.
    CHARGING_MODE_OVERHEATING,

    // Постоянная (принудительная) зарядка вне зависимости от тарифа.
    CHARGING_MODE_ALWAYS,
};

// Событие: тарифный период изменился. Машина переводит зарядку в состояние,
// соответствующее новому тарифу.
struct TariffEvent : tinyfsm::Event {
    // Новый тарифный период.
    TimeInterval tariff;
};

// Предварительные объявления состояний зарядки.
struct ChargingOffState;
struct MinimalPowerChargingState;
struct MaxPowerChargingState;
struct OverheatingState;
struct AlwaysChargingState;

// Базовый класс стейт-машины устройства. Обработчик смены тарифа общий для
// всех состояний (таблица переходов в device_state_machine.cpp); каждое
// состояние обязано определить собственное действие входа entry() и может
// переопределить реакцию на тариф (например, состояние перегрева игнорирует
// тариф).
struct DeviceStateMachine : tinyfsm::Fsm<DeviceStateMachine> {
    virtual void react(TariffEvent const&);
    virtual void entry() = 0;
    virtual void exit() {}
};

// Запускает стейт-машину устройства: выполняет вход в начальное состояние.
void startDeviceStateMachine();

// Сообщает машине о новом тарифном периоде: зарядка переводится в
// соответствующее состояние (таблица переходов в device_state_machine.cpp).
void setTariff(TimeInterval tariff);

// Возвращает режим зарядки, соответствующий текущему состоянию машины.
ChargingMode getChargingMode();

// Обновляет текущий режим зарядки; вызывается только действиями входа
// состояний.
void setActiveChargingMode(ChargingMode mode);
