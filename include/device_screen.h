#pragma once

#include "display.h"
#include "relays.h"
#include "time_switcher.h"

// Состояние устройства, выводимое на дисплей.
struct DeviceState {
    // Состояние реле.
    RelayState relays;

    // Тарифный период по выключателю (D6/D7).
    TimeInterval time_interval_type;

    // Сравнивает два состояния на равенство.
    bool operator==(const DeviceState& other) const {
        return relays == other.relays && time_interval_type == other.time_interval_type;
    }
};

class DeviceScreen {
public:
    // Инициализирует дисплей и запоминает ссылку на него для вывода состояний.
    void init(Display& display);

    // Выводит состояния реле (ON/OFF) одной строкой: каждое слово занимает фиксированные 4 символа.
    void print_state(const DeviceState& state);

private:
    // Указатель на инициализированный дисплей.
    Display* m_display;
};
