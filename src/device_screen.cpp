#include "device_screen.h"

// Статический буфер строки состояния реле (длина не превышает Display::LINE_BUF).
static char STATE_LINE[Display::LINE_BUF];

// Статический буфер обозначения тарифного периода (3 символа + '\0').
static char INTERVAL_LINE[4];

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

  // Обозначение активного тарифного периода (3 символа) для второй строки.
  if (state.time_interval_type == FORCE_CHARGING) {
    // включена принудительная зарядка
    memcpy(INTERVAL_LINE, "frc", 3u);
  } else if (state.time_interval_type == MIDDLE) {
    // средняя цена электричества
    memcpy(INTERVAL_LINE, "mdl", 3u);
  } else if (state.time_interval_type == EXPENSIVE) {
    // дорогая цена электричества
    memcpy(INTERVAL_LINE, "exp", 3u);
  } else {
    // дешевая цена электричества
    memcpy(INTERVAL_LINE, "chp", 3u);
  }
  INTERVAL_LINE[3] = '\0';

  // Выводим состояние реле на первую строку и тарифный период на вторую.
  m_display->print(STATE_LINE, INTERVAL_LINE);
}
