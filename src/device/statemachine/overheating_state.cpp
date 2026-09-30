#include "overheating_state.h"

// Действие входа: перегрев — зарядка останавливается, в боевой реализации
// здесь выключаются все реле зарядок независимо от тарифа.
void OverheatingState::entry() {
  setActiveChargingMode(CHARGING_MODE_OVERHEATING);
}

// Смена тарифа игнорируется, пока активен перегрев: состояние защищает
// устройство и не передаёт управление тарифной логике.
void OverheatingState::react(TariffEvent const&) {
}
