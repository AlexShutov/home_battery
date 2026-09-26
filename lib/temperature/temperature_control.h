#pragma once

#include <Arduino.h>
#include "ir_sensor.h"

// Колбэк контроля температуры: без параметров, вызывается по факту события.
typedef void (*TemperatureCallback)();

// Состояние контроля температуры по четырём IR-датчикам.
struct TemperatureControlState {
    // Количество датчиков.
    static const uint8_t NUM_SENSORS = 4;

    // Последние показания датчиков, °C (NAN — данных нет).
    float temperatures[NUM_SENSORS];

    // Выходит ли показание датчика за пределы своих min/max порогов.
    bool outOfRange[NUM_SENSORS];

    // Все ли датчики остыли ниже минимальных порогов.
    bool allCooledDown;

    // Все ли датчики перегреты выше максимальных порогов.
    bool allTooHot;

};

// Контроль температуры по четырём IR-датчикам MLX90614: опрашивает датчики,
// следит за выходом показаний за min/max пороги и оповещает колбэками, когда
// все датчики сразу остыли ниже минимума или перегреты выше максимума.
class TemperatureControl {
public:
    // Количество датчиков.
    static const uint8_t NUM_SENSORS = TemperatureControlState::NUM_SENSORS;

    // Адреса датчиков на шине I2C.
    static const uint8_t SENSOR_1_ADDRESS = 0x5A;
    static const uint8_t SENSOR_2_ADDRESS = 0x5B;
    static const uint8_t SENSOR_3_ADDRESS = 0x5C;
    static const uint8_t SENSOR_4_ADDRESS = 0x5D;

    // Пороги датчика 1, °C.
    static const int8_t SENSOR_1_MIN_THRESHOLD_TEMP = 40;
    static const int8_t SENSOR_1_MAX_THRESHOLD_TEMP = 75;

    // Пороги датчика 2, °C.
    static const int8_t SENSOR_2_MIN_THRESHOLD_TEMP = 40;
    static const int8_t SENSOR_2_MAX_THRESHOLD_TEMP = 75;

    // Пороги датчика 3, °C.
    static const int8_t SENSOR_3_MIN_THRESHOLD_TEMP = 40;
    static const int8_t SENSOR_3_MAX_THRESHOLD_TEMP = 50;

    // Пороги датчика 4, °C.
    static const int8_t SENSOR_4_MIN_THRESHOLD_TEMP = 40;
    static const int8_t SENSOR_4_MAX_THRESHOLD_TEMP = 75;

    // Конструктор: колбэки сброшены, показаний нет.
    TemperatureControl();

    // Подключает все датчики по их адресам и запоминает колбэки:
    // onCooledDown — все датчики остыли ниже минимальных порогов;
    // onTempTooHigh — все датчики перегреты выше максимальных порогов.
    void init(TemperatureCallback onCooledDown, TemperatureCallback onTempTooHigh);

    // Опрашивает все датчики, проверяет нахождение показаний в пределах min/max
    // и при переходе в состояние полного остывания/перегрева вызывает колбэк.
    void loop();

    // Возвращает состояние контроля в выходной параметр.
    void getState(TemperatureControlState& out) const;

    // Фиксирует состояние; управляемой аппаратной части нет (датчики — источники).
    void setState(const TemperatureControlState& newState);

private:
    // Минимальные пороги по индексу датчика, °C.
    static const int8_t MIN_THRESHOLD_TEMPS[NUM_SENSORS];

    // Максимальные пороги по индексу датчика, °C.
    static const int8_t MAX_THRESHOLD_TEMPS[NUM_SENSORS];

    // Инициализирует датчик по индексу с заданным адресом.
    void initSensor(uint8_t index, uint8_t address);

    // IR-датчики температуры.
    IrSensor sensors[NUM_SENSORS];

    // Состояние контроля.
    TemperatureControlState state;

    // Колбэк полного остывания (все датчики ниже минимума).
    TemperatureCallback onCooledDown;

    // Колбэк перегрева (все датчики выше максимума).
    TemperatureCallback onTempTooHigh;
};
