#include "time_switcher.h"

const uint8_t TimeSwitcher::PIN_MIDDLE_PERIOD;
const uint8_t TimeSwitcher::PIN_EXPENSIVE_PERIOD;

void TimeSwitcher::init() {
  pinMode(PIN_MIDDLE_PERIOD, INPUT);
  pinMode(PIN_EXPENSIVE_PERIOD, INPUT);
  readState();
}

void TimeSwitcher::readState() {
  state.isMiddlePeriodActive = (digitalRead(PIN_MIDDLE_PERIOD) == HIGH);
  state.isExpensivePeriodActive = (digitalRead(PIN_EXPENSIVE_PERIOD) == HIGH);
}

void TimeSwitcher::getState(TimeSwitcherState& out) {
  readState();
  out = state;
}

void TimeSwitcher::setState(const TimeSwitcherState& newState) {
  state = newState;
}
