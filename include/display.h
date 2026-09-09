#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Display {
public:
    // Максимальная длина строки в статических буферах вывода.
    // Должна быть не меньше количества колонок дисплея.
    static const uint8_t LINE_BUF = 20;

    Display(uint8_t address = 0x27, uint8_t cols = 16, uint8_t rows = 2);

    void init();

    // Выводит две строки. Кириллические символы (UTF-8 или cp866)
    // преобразуются в глифы через CyrillicMapper и выводятся на дисплей.
    void print(const String& firstString, const String& secondString);

    // Выводит две строки из байтовых массивов, полученных из CyrillicMapper::mapToCyril.
    // Массивы завершаются нулём; байты >= 0x80 считаются индексами глифов,
    // остальные байты выводятся как есть.
    void print(const uint8_t* firstByteArray, const uint8_t* secondByteArray = nullptr);

private:
    void writeLine(const uint8_t* text, uint8_t row);

    LiquidCrystal_I2C lcd;
    uint8_t lineBufFirst[LINE_BUF + 1];
    uint8_t lineBufSecond[LINE_BUF + 1];
    uint8_t cols;
    uint8_t rows;
};
