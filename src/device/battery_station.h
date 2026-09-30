#pragma once

#include "device_logic.h"
#include "device/battery/battery_reader.h"
#include "statemachine/device_state_machine.h"

// Состояние аккумуляторной станции: тарифный период и режим зарядки.
struct BatteryStationState {
    // Тарифный период по выключателю (D6/D7).
    TimeInterval time_interval_type;

    // Режим зарядки, установленный стейт-машиной.
    ChargingMode charging_mode;

    // Сравнивает два состояния на равенство.
    bool operator==(const BatteryStationState& other) const {
        return time_interval_type == other.time_interval_type &&
               charging_mode == other.charging_mode;
    }
};

// Логика аккумуляторной станции: строится вокруг стейт-машины зарядки,
// состояния которой лежат в src/device/statemachine.
class BatteryStation : public DeviceLogic {
public:
    // Инициализирует компоненты устройства и запускает стейт-машину зарядки.
    void init() override;

    // Главный цикл: опрашивает состояние и реагирует на его изменения.
    void loop() override;

    // Считывает актуальное состояние станции в выходной параметр.
    void getState(BatteryStationState& out);

    // Сообщает стейт-машине тарифный период из переданного состояния: зарядка
    // переводится в соответствующий режим.
    void setState(const BatteryStationState& state);

protected:
    // Реакция на изменение состояния устройства (смена тарифного периода):
    // сообщает стейт-машине новый тариф.
    void updateState() override;

private:
    // Опрашивает БМС и обновляет снимок показаний батареи в стейт-машине.
    void updateBatterySnapshot();

    // Показания батареи для передачи в стейт-машину.
    BatteryState battery_state;
};
