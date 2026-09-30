#include "test_charging_fsm.h"

#include <tinyfsm.hpp>

// Число включённых зарядок в текущем состоянии машины (0..4). Обновляется
// только действиями входа состояний. В боевой реализации сюда же отправляется
// команда реле (relays.setState()), здесь железо не трогается.
static uint8_t active_chargers = 0;

// Событие: перейти к следующему состоянию зарядок по циклу.
struct NextStateEvent : tinyfsm::Event {};

// Базовый класс стейт-машины зарядок: каждое состояние обязано обработать
// событие перехода и определить действие входа.
struct ChargingFsm : tinyfsm::Fsm<ChargingFsm> {
    virtual void react(NextStateEvent const&) = 0;
    virtual void entry() = 0;
    virtual void exit() {}
};

// Предварительные объявления состояний.
struct ChargingOff;
struct ChargingOne;
struct ChargingTwo;
struct ChargingThree;
struct ChargingAll;

// Зарядки выключены.
struct ChargingOff : ChargingFsm {
    void entry() override { active_chargers = 0; }
    void react(NextStateEvent const&) override { transit<ChargingOne>(); }
};

// Включена одна зарядка.
struct ChargingOne : ChargingFsm {
    void entry() override { active_chargers = 1; }
    void react(NextStateEvent const&) override { transit<ChargingTwo>(); }
};

// Включены две зарядки.
struct ChargingTwo : ChargingFsm {
    void entry() override { active_chargers = 2; }
    void react(NextStateEvent const&) override { transit<ChargingThree>(); }
};

// Включены три зарядки.
struct ChargingThree : ChargingFsm {
    void entry() override { active_chargers = 3; }
    void react(NextStateEvent const&) override { transit<ChargingAll>(); }
};

// Включены все зарядки.
struct ChargingAll : ChargingFsm {
    void entry() override { active_chargers = 4; }
    void react(NextStateEvent const&) override { transit<ChargingOff>(); }
};

// Начальное состояние — зарядки выключены.
FSM_INITIAL_STATE(ChargingFsm, ChargingOff)

// Событие перехода существует в единственном экземпляре в глобальной памяти:
// локальные объекты неинтегральных типов в функциях создавать нельзя.
static const NextStateEvent next_state_event;

// Запускает стейт-машину зарядок. Вызов статического метода библиотеки
// обёрнут в свободную функцию, как того требуют правила проекта.
static void startChargingFsm() {
    ChargingFsm::start();
}

// Отправляет машине событие перехода к следующему состоянию.
static void dispatchNextStateEvent() {
    ChargingFsm::dispatch(next_state_event);
}

// Возвращает число включённых зарядок в текущем состоянии машины.
uint8_t getActiveChargers() {
    return active_chargers;
}

void TestChargingFsm::init() {
    startChargingFsm();
}

void TestChargingFsm::loop() {
    ++tick_counter_;
    if (tick_counter_ >= TICKS_PER_EVENT) {
        tick_counter_ = 0;
        dispatchNextStateEvent();
    }
}
