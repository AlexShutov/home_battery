#pragma once

#include "charge_control_state.h"
#include "device/battery/battery_state.h"
#include "relays.h"

// Контроль зарядок: рассчитывает целевое состояние зарядок по показаниям
// батареи и применяет его к реле. Каждое реле коммутирует зарядку с
// собственным номинальным током (см. charger_currents.h).
class ChargeControl {
public:
    // Инициализирует реле зарядок (все выключены).
    void init();

    // Рассчитывает целевое состояние зарядок по показаниям батареи
    // (обёртка над свободной функцией calculateTargetState — расчёт не
    // зависит от состояния объекта и тестируется в native-окружении).
    void calculateTargetState(const BatteryState& battery,
                              ChargeControlState& out,
                              bool isMinimalCurrent);

    // Применяет целевое состояние к зарядкам: при перегреве выключаются
    // все, иначе включаются зарядки с флагом 1 в activeRelays.
    void activateTargetState(const BatteryState& battery, const ChargeControlState& target);

    // Считывает последнее применённое состояние зарядок.
    void getState(ChargeControlState& out) const;

    // Устанавливает состояние зарядок: обновляет внутреннее состояние и
    // переключает реле согласно флагам activeRelays. Температура здесь не
    // проверяется — за неё отвечает activateTargetState().
    void setState(const ChargeControlState& newState);

    // Считывает фактическое состояние реле зарядок (для экрана станции).
    void getRelayState(RelayState& out) const;

private:
    // Реле зарядок.
    Relays relays;

    // Последнее применённое состояние зарядок.
    ChargeControlState state;
};
