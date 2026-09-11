#pragma once

#include <Arduino.h>

// Состояние двухканального выключателя умного дома (ножки D6/D7).
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

class TimeSwitcher {
public:
    // Инициализирует ножки D6/D7 как входы с внутренними подтягивающими резисторами.
    void init();

    // Возвращает актуальное состояние выключателя во входной параметр (ножки считываются при каждом вызове).
    void getState(TimeSwitcherState& out);

private:
    // Ножка промежуточного периода (D6).
    static const uint8_t PIN_MIDDLE_PERIOD = 6;

    // Ножка дорогого периода (D7).
    static const uint8_t PIN_EXPENSIVE_PERIOD = 7;
};
