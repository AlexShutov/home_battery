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

// Предварительные объявления: контроль зарядок и показания батареи.
class ChargeControl;
struct BatteryState;

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

// Привязывает контроль зарядок к машине: действия состояний рассчитывают
// и применяют целевое состояние зарядок. Вызывается до запуска машины;
// до привязки действия состояний зарядки не меняют.
void bindChargeControl(ChargeControl& control);

// Обновляет снимок показаний батареи, по которому состояния рассчитывают
// целевое состояние зарядок; станция кладёт свежие показания перед
// отправкой тарифного события.
void setBatterySnapshot(const BatteryState& battery);

// Привязанный контроль зарядок: nullptr, пока машина не связана с
// устройством.
ChargeControl* getChargeControl();

// Снимок показаний батареи для расчётов состояний.
const BatteryState& getBatterySnapshot();

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
