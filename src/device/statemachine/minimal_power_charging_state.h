#pragma once

#include "device_state_machine.h"

// Состояние «зарядка включена на минимальную мощность»: активна только часть
// зарядок. Соответствует промежуточному тарифному периоду (MIDDLE).
struct MinimalPowerChargingState : DeviceStateMachine {
    // Действие входа: фиксирует режим зарядки.
    void entry() override;
};
