#pragma once

#include <Arduino.h>

// Тарифный период, определённый выключателем тарифов (ножки D6/D7).
enum TimeInterval : uint8_t {
    // Дешёвый период: промежуточный и дорогой периоды не активны.
    CHEAP = 0,

    // Промежуточный период (ножка D6).
    MIDDLE,

    // Дорогой период (ножка D7).
    EXPENSIVE
};

class TimeSwitcher {
public:
    // Инициализирует ножки D6/D7 как входы с внутренними подтягивающими резисторами.
    void init();

    // Считывает ножки и возвращает активный тарифный период.
    TimeInterval getState();

private:
    // Внутреннее состояние двухканального выключателя умного дома (ножки D6/D7).
    struct TimeSwitcherState {
        // Активен ли промежуточный период (ножка D6).
        bool isMiddlePeriodActive;

        // Активен ли дорогой период (ножка D7).
        bool isExpensivePeriodActive;

        // Сравнивает два состояния на равенство.
        bool operator==(const TimeSwitcherState& other) const {
            return isMiddlePeriodActive == other.isMiddlePeriodActive &&
                   isExpensivePeriodActive == other.isExpensivePeriodActive;
        }
    };

    // Ножка промежуточного периода (D6).
    static const uint8_t PIN_MIDDLE_PERIOD = 6;

    // Ножка дорогого периода (D7).
    static const uint8_t PIN_EXPENSIVE_PERIOD = 7;

    // Внутреннее состояние выключателя.
    TimeSwitcherState state;
};
