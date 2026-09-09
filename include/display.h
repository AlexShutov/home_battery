#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Display {
public:
    Display(uint8_t address = 0x27, uint8_t cols = 16, uint8_t rows = 2);

    void init();

    void print(const String& firstString, const String& secondString = String());

private:
    LiquidCrystal_I2C lcd;
    uint8_t cols;
    uint8_t rows;
};
