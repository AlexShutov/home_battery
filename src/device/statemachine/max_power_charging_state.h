#pragma once

#include "device_state_machine.h"

// Состояние «зарядка включена на максимальную мощность»: активны все
// зарядки. Соответствует дешёвому тарифному периоду (CHEAP).
struct MaxPowerChargingState : DeviceStateMachine {
    // Действие входа: фиксирует режим зарядки.
    void entry() override;
};
