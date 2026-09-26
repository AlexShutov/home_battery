#include "display.h"

const uint8_t Display::LINE_BUF;

Display::Display(uint8_t address, uint8_t cols, uint8_t rows)
    : lcd(address, cols, rows), cols(cols), rows(rows) {}

// В форке johnrickman/LiquidCrystal_I2C Wire.begin() и флаги режима дисплея
// (_displayfunction с LCD_4BITMODE) выставляются только в init(); begin() без
// init() шлёт команды по несконфигурированному TWI — экран остаётся пустым.
// init() сам вызывает begin() с размерами из конструктора (16x2).
void Display::init() { lcd.init(); lcd.backlight(); }

void Display::print(const char* firstString, const char* secondString) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(firstString);
  lcd.setCursor(0, 1);
  lcd.print(secondString);
}