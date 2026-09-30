#include "always_charging_state.h"

// Действие входа: постоянная зарядка — в боевой реализации здесь включаются
// зарядки постоянного режима (состав будет определён при настройке мощности,
// пока фиксируется только режим).
void AlwaysChargingState::entry() {
  setActiveChargingMode(CHARGING_MODE_ALWAYS);
}
