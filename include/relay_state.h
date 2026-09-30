#pragma once

#include <stdint.h>

// Состояние четырёх реле.
struct RelayState {
    // Количество реле.
    static const uint8_t NUM_RELAYS = 4;

    // Состояние каждого реле: true — включено.
    bool relays[NUM_RELAYS];

    // Сравнивает два состояния на равенство.
    bool operator==(const RelayState& other) const {
        for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
            if (relays[i] != other.relays[i]) {
                return false;
            }
        }
        return true;
    }
};
