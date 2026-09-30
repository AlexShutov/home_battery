#pragma once

#include "device_state_machine.h"

// Состояние «постоянная зарядка»: зарядка включена непрерывно, вне
// зависимости от тарифа. Соответствует принудительному режиму выключателя
// тарифов (FORCE_CHARGING, оба выключателя включены).
struct AlwaysChargingState : DeviceStateMachine {
    // Действие входа: фиксирует режим зарядки и включает все реле зарядок.
    void entry() override;

    // Действие выхода: выключает все реле зарядок.
    void exit() override;
};
