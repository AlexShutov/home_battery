#pragma once

#include <stdint.h>

#include "relay_state.h"

// Состояние контроля зарядок: общий флаг зарядки и набор включённых зарядок.
struct ChargeControlState {
    // Зарядка разрешена: температура в норме и есть что включать.
    bool isChargeOn;

    // Флаги включённых зарядок: 1 — включена, 0 — выключена. Размер равен
    // числу реле; зарядки с нулевым номиналом (резерв) не включаются.
    uint8_t activeRelays[RelayState::NUM_RELAYS];

    // Сравнивает два состояния на равенство.
    bool operator==(const ChargeControlState& other) const {
        if (isChargeOn != other.isChargeOn) {
            return false;
        }
        for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
            if (activeRelays[i] != other.activeRelays[i]) {
                return false;
            }
        }
        return true;
    }
};
