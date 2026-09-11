#include "time_switcher.h"

const uint8_t TimeSwitcher::PIN_MIDDLE_PERIOD;
const uint8_t TimeSwitcher::PIN_EXPENSIVE_PERIOD;

void TimeSwitcher::init() {
  pinMode(PIN_MIDDLE_PERIOD, INPUT_PULLUP);
  pinMode(PIN_EXPENSIVE_PERIOD, INPUT_PULLUP);
}

// Активность периода определяется уровнем LOW (входы подтянуты на высокий уровень).
void TimeSwitcher::getState(TimeSwitcherState& out) {
  out.isMiddlePeriodActive = (digitalRead(PIN_MIDDLE_PERIOD) == LOW);
  out.isExpensivePeriodActive = (digitalRead(PIN_EXPENSIVE_PERIOD) == LOW);
}
