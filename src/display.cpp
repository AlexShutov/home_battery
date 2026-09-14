#include "display.h"

const uint8_t Display::LINE_BUF;

Display::Display(uint8_t address, uint8_t cols, uint8_t rows)
    : lcd(address, cols, rows), cols(cols), rows(rows) {}

void Display::init() { lcd.begin(cols, rows); lcd.backlight(); }

void Display::print(const char* firstString, const char* secondString) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(firstString);
  lcd.setCursor(0, 1);
  lcd.print(secondString);
}