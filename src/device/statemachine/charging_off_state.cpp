#include "charging_off_state.h"

// Действие входа: зарядка выключена — в боевой реализации здесь выключаются
// все реле зарядок (через setState реле).
void ChargingOffState::entry() {
  setActiveChargingMode(CHARGING_MODE_OFF);
}
