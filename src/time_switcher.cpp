#include "time_switcher.h"

const uint8_t TimeSwitcher::PIN_MIDDLE_PERIOD;
const uint8_t TimeSwitcher::PIN_EXPENSIVE_PERIOD;

void TimeSwitcher::init() {
  pinMode(PIN_MIDDLE_PERIOD, INPUT_PULLUP);
  pinMode(PIN_EXPENSIVE_PERIOD, INPUT_PULLUP);
}

// Активность периода определяется уровнем LOW (входы подтянуты на высокий уровень).
TimeInterval TimeSwitcher::getState() {
  state.isMiddlePeriodActive = (digitalRead(PIN_MIDDLE_PERIOD) == LOW);
  state.isExpensivePeriodActive = (digitalRead(PIN_EXPENSIVE_PERIOD) == LOW);

  if (state.isMiddlePeriodActive) {
    return MIDDLE;
  }
  if (state.isExpensivePeriodActive) {
    return EXPENSIVE;
  }
  return CHEAP;
}
