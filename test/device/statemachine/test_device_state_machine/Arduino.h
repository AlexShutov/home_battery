// Заглушка Arduino.h для нативных тестов стейт-машины зарядки.
// time_switcher.h включает <Arduino.h>, а relays.cpp пишет в регистры
// порта D AVR (ножки D2–D5 реле зарядок). Регистры объявлены extern и
// определяются в файле теста обычными переменными в памяти — так тесты
// проверяют и логическое состояние реле, и уровни ножек.
#ifndef ARDUINO_H_NATIVE_STUB
#define ARDUINO_H_NATIVE_STUB

#include <stdint.h>

// Регистры направления данных и выхода порта D AVR.
extern uint8_t DDRD;
extern uint8_t PORTD;

#endif // ARDUINO_H_NATIVE_STUB
