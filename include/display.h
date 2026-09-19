#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Display {
public:
    // Максимальная длина строки в статических буферах вывода.
    static const uint8_t LINE_BUF = 20;

    Display(uint8_t address = 0x27, uint8_t cols = 16, uint8_t rows = 2);

    void init();

    // Выводит две C-строки в кодировке дисплея (ASCII); String не используется,
    // чтобы не выделять память динамически.
    void print(const char* firstString, const char* secondString);

private:
    LiquidCrystal_I2C lcd;
    uint8_t cols;
    uint8_t rows;
};