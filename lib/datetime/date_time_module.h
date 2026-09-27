#pragma once

#include <Arduino.h>
#include <RTClib.h>

// Колбэк результата инициализации модуля времени: без параметров.
typedef void (*DateTimeCallback)();

// Состояние модуля времени DS3231 (последнее прочитанное показание).
struct DateTimeModuleState {
    // Год.
    uint16_t year;

    // Месяц 1..12.
    uint8_t month;

    // День 1..31.
    uint8_t day;

    // Часы 0..23.
    uint8_t hour;

    // Минуты 0..59.
    uint8_t minute;

    // Секунды 0..59.
    uint8_t second;

    // Успешна ли инициализация (модуль отвечает, время валидно).
    bool connected;

    // Сравнивает два состояния на равенство.
    bool operator==(const DateTimeModuleState& other) const {
        return year == other.year && month == other.month && day == other.day &&
               hour == other.hour && minute == other.minute && second == other.second &&
               connected == other.connected;
    }
};

// Модуль реального времени RTC DS3231 TZT: инициализация, чтение и запись
// времени. В качестве модели даты/времени используется класс DateTime
// библиотеки RTClib (Adafruit).
class DateTimeModule {
public:
    // Конструктор: модуль ещё не инициализирован, время обнулено.
    DateTimeModule();

    // Инициализирует DS3231 на шине I2C: begin() проверяет отклик чипа,
    // lostPower() — валидность времени (флаг поднимается при севшей резервной
    // батарее). При успехе вызывает onInitOk, при неудаче — onInitFailed.
    // Возвращает true при успешной инициализации.
    bool init(DateTimeCallback onInitOk, DateTimeCallback onInitFailed);

    // Читает текущее время модуля в модель DateTime (библиотека RTClib)
    // и обновляет состояние.
    void getTime(DateTime& out);

    // Записывает новое время в модуль (модель DateTime библиотеки RTClib).
    void setTime(const DateTime& newTime);

    // Возвращает состояние модуля в выходной параметр.
    void getState(DateTimeModuleState& out) const;

    // Фиксирует состояние; аппаратная запись времени выполняется методом
    // setTime() с моделью DateTime библиотеки.
    void setState(const DateTimeModuleState& newState);

private:
    // Модуль RTC DS3231 (адрес 0x68 на шине I2C).
    RTC_DS3231 rtc;

    // Состояние модуля.
    DateTimeModuleState state;
};
