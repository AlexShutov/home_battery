#include "time_switcher.h"

const uint8_t TimeSwitcher::PIN_MIDDLE_PERIOD;
const uint8_t TimeSwitcher::PIN_EXPENSIVE_PERIOD;

void TimeSwitcher::init() {
  pinMode(PIN_MIDDLE_PERIOD, INPUT);
  pinMode(PIN_EXPENSIVE_PERIOD, INPUT);
}

void TimeSwitcher::getState(TimeSwitcherState& out) {
  out.isMiddlePeriodActive = (digitalRead(PIN_MIDDLE_PERIOD) == HIGH);
  out.isExpensivePeriodActive = (digitalRead(PIN_EXPENSIVE_PERIOD) == HIGH);
}
