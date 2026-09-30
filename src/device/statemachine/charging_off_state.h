#pragma once

#include "device_state_machine.h"

// Состояние «зарядка выключена»: зарядки не активны. Соответствует дорогому
// тарифному периоду (EXPENSIVE) — зарядка в это время невыгодна.
struct ChargingOffState : DeviceStateMachine {
    // Действие входа: фиксирует режим зарядки.
    void entry() override;
};
