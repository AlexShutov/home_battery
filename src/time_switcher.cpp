#include "time_switcher.h"

const uint8_t TimeSwitcher::PIN_MIDDLE_PERIOD;
const uint8_t TimeSwitcher::PIN_EXPENSIVE_PERIOD;
const uint8_t TimeSwitcher::DEBOUNCE_SAMPLES;
const uint16_t TimeSwitcher::DEBOUNCE_DELAY_MS;
const uint8_t TimeSwitcher::DEBOUNCE_MAX_ATTEMPTS;

// Возвращает уровень ножки (true — высокий уровень). Служит колбэком для readDebounced.
static bool readPinLevel(uint8_t pin) {
  return digitalRead(pin);
}

void TimeSwitcher::init() {
  pinMode(PIN_MIDDLE_PERIOD, INPUT_PULLUP);
  pinMode(PIN_EXPENSIVE_PERIOD, INPUT_PULLUP);
}

// Гасит дребезг контакта: опрашивает ножку через колбэк до DEBOUNCE_SAMPLES одинаковых
// подряд срабатываний, но суммарно не дольше DEBOUNCE_MAX_ATTEMPTS опросов. Если вход
// бесконечно меняет уровень (шум/осцилляция), возвращается последний прочитанный уровень.
bool TimeSwitcher::readDebounced(uint8_t pin, bool (*readLevel)(uint8_t)) {
  // Первое срабатывание задаёт уровень, который нужно подтвердить повторными опросами.
  bool level = readLevel(pin);

  uint8_t stableCount = 1;
  uint8_t attempts = 1;
  while (stableCount < DEBOUNCE_SAMPLES && attempts < DEBOUNCE_MAX_ATTEMPTS) {
    delay(DEBOUNCE_DELAY_MS);

    ++attempts;
    bool next = readLevel(pin);
    if (next != level) {
      // Контакт дребезжит: фиксируем новый уровень и начинаем отсчёт заново.
      level = next;
      stableCount = 1;
    } else {
      ++stableCount;
    }
  }

  return level;
}

// Читает ножку промежуточного периода с гашением дребезга. Период активен при уровне LOW (вход подтянут на высокий уровень).
bool TimeSwitcher::readMiddlePeriodPin() {
  return readDebounced(PIN_MIDDLE_PERIOD, readPinLevel);
}

// Читает ножку дорогого периода с гашением дребезга. Период активен при уровне LOW (вход подтянут на высокий уровень).
bool TimeSwitcher::readExpensivePeriodPin() {
  return readDebounced(PIN_EXPENSIVE_PERIOD, readPinLevel);
}

TimeInterval TimeSwitcher::getState() {
  state.isMiddlePeriodActive = !readMiddlePeriodPin();
  state.isExpensivePeriodActive = !readExpensivePeriodPin();

  if (state.isMiddlePeriodActive && state.isExpensivePeriodActive) {
    return FORCE_CHARGING;
  }

  if (state.isMiddlePeriodActive) {
    return MIDDLE;
  }

  if (state.isExpensivePeriodActive) {
    return EXPENSIVE;
  }

  return CHEAP;
}
