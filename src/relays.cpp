#include "relays.h"

const uint8_t Relays::NUM_RELAYS;
const uint8_t Relays::RELAY_1;
const uint8_t Relays::RELAY_2;
const uint8_t Relays::RELAY_3;
const uint8_t Relays::RELAY_4;
const uint8_t Relays::PIN_MASK;
const uint8_t Relays::FIRST_PIN_BIT;

Relays::Relays() {}

void Relays::init() {
  DDRD |= PIN_MASK;
  PORTD |= PIN_MASK;
  for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
    state.relays[i] = false;
  }
}

void Relays::turnAllOn() {
  PORTD &= (uint8_t)~PIN_MASK;
  for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
    state.relays[i] = true;
  }
}

void Relays::turnAllOff() {
  PORTD |= PIN_MASK;
  for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
    state.relays[i] = false;
  }
}

void Relays::turnOn(uint8_t relay) {
  setRelay(relay, true);
}

void Relays::turnOff(uint8_t relay) {
  setRelay(relay, false);
}

void Relays::setRelay(uint8_t relay, bool on) {
  if (relay < RELAY_1 || relay > RELAY_4) {
    return;
  }

  uint8_t bit = (uint8_t)(1u << (relay + FIRST_PIN_BIT));

  if (on) {
    PORTD &= (uint8_t)~bit;
  } else {
    PORTD |= bit;
  }

  state.relays[relay] = on;
}

void Relays::setState(const RelayState& newState) {
  for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
    setRelay(i, newState.relays[i]);
  }
}

void Relays::getState(RelayState& out) const {
  out = state;
}