#include "minimal_power_charging_state.h"

// Действие входа: минимальная мощность — в боевой реализации здесь включается
// часть зарядок (состав будет определён при настройке мощности, пока
// фиксируется только режим).
void MinimalPowerChargingState::entry() {
  setActiveChargingMode(CHARGING_MODE_MINIMAL);
}
