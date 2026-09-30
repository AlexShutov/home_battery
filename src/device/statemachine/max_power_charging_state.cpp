#include "max_power_charging_state.h"

// Действие входа: максимальная мощность — в боевой реализации здесь
// включаются все зарядки (через setState реле).
void MaxPowerChargingState::entry() {
  setActiveChargingMode(CHARGING_MODE_MAX);
}
