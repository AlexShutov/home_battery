#pragma once

#include <Arduino.h>

// Тарифный период, определённый выключателем тарифов (ножки D6/D7).
enum TimeInterval : uint8_t {
    // Дешёвый период: промежуточный и дорогой периоды не активны.
    CHEAP = 0,

    // Промежуточный период (ножка D6).
    MIDDLE,

    // Дорогой период (ножка D7).
    EXPENSIVE,

    // оба выключателя включены, принудительно включается зарядка
    FORCE_CHARGING,
};

class TimeSwitcher {
public:
    // Инициализирует ножки D6/D7 как входы с внутренними подтягивающими резисторами.
    void init();

    // Считывает ножки и возвращает активный тарифный период.
    TimeInterval getState();

protected:
    // Гасит дребезг контакта: опрашивает ножку через колбэк до DEBOUNCE_SAMPLES одинаковых подряд срабатываний.
    bool readDebounced(uint8_t pin, bool (*readLevel)(uint8_t));

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

    // Количество одинаковых подряд идущих опросов для гашения дребезга контакта.
    static const uint8_t DEBOUNCE_SAMPLES = 4;

    // Задержка между опросами ножки при гашении дребезга, мс.
    static const uint16_t DEBOUNCE_DELAY_MS = 5;

    // Максимальное общее число опросов ножки при гашении дребезга: защита от
    // вечного цикла, если вход бесконечно меняет уровень (шум/осцилляция).
    static const uint8_t DEBOUNCE_MAX_ATTEMPTS = 20;

    // Читает ножку промежуточного периода с гашением дребезга; true — период активен (LOW).
    bool readMiddlePeriodPin();

    // Читает ножку дорогого периода с гашением дребезга; true — период активен (LOW).
    bool readExpensivePeriodPin();

    // Внутреннее состояние выключателя.
    TimeSwitcherState state;
};
