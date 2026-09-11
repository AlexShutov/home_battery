#include "device_screen.h"

// Статический буфер строки состояния реле (длина не превышает Display::LINE_BUF).
static char STATE_LINE[Display::LINE_BUF];

void DeviceScreen::init(Display& display) {
  m_display = &display;
  m_display->init();
}

void DeviceScreen::print_state(const DeviceState& state) {
  // Собираем строку вида "ON  OFF ON  OFF": каждое реле занимает фиксированные 4 символа
  // (слово + пробел-разделитель), чтобы слова не сливались и позиции не сдвигались.
  uint8_t pos = 0;
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    if (state.relays.relays[i]) {
      memcpy(&STATE_LINE[pos], "ON  ", 4u);
    } else {
      memcpy(&STATE_LINE[pos], "OFF ", 4u);
    }
    pos += 4u;
  }
  STATE_LINE[pos] = '\0';

  // Выводим состояние на первую строку, вторую оставляем пустой.
  m_display->print(STATE_LINE, "");
}
