#pragma once

#include "display.h"
#include "relays.h"

// Состояние устройства, выводимое на дисплей.
struct DeviceState {
    // Состояние реле.
    RelayState relays;
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
