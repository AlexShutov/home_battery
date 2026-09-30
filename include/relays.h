#pragma once

#include <stdint.h>

#include "device/statemachine/charging_relays_port.h"
#include "relay_state.h"

// Реле зарядок станции. Реализует порт стейт-машины зарядки: машина
// управляет реле через setState().
class Relays : public ChargingRelaysPort {
public:
    // Количество реле.
    static const uint8_t NUM_RELAYS = RelayState::NUM_RELAYS;

    // Номера реле (с нуля).
    static const uint8_t RELAY_1 = 0;
    static const uint8_t RELAY_2 = 1;
    static const uint8_t RELAY_3 = 2;
    static const uint8_t RELAY_4 = 3;

    Relays();

    // Инициализирует ножки как выходы с низким уровнем (реле выключены).
    void init();

    // Включает все реле.
    void turnAllOn();

    // Выключает все реле.
    void turnAllOff();

    // Включает реле по номеру (0..3).
    void turnOn(uint8_t relay);

    // Выключает реле по номеру (0..3).
    void turnOff(uint8_t relay);

    // Устанавливает состояние реле по данным из массива (реализация порта
    // стейт-машины зарядки).
    void setState(const RelayState& newState) override;

    // Возвращает текущее состояние реле в выходной параметр.
    void getState(RelayState& out) const;

private:
    // Маска пинов реле: биты 2..5 порта D (ножки D2–D5).
    static const uint8_t PIN_MASK = (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);

    // Номер бита порта D для реле 0 (ножка D2).
    static const uint8_t FIRST_PIN_BIT = 2;

    // Устанавливает состояние одного реле по номеру.
    void setRelay(uint8_t relay, bool on);

    RelayState state;
};