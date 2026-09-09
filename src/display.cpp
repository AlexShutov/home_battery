#include "display.h"
#include "cyrillic_mapper.h"

const uint8_t Display::LINE_BUF;

// Экземпляр маппера в статическом хранилище (метод не зависит от состояния объекта).
static const CyrillicMapper mapper;

// Буфер битовой схемы глифа для LiquidCrystal_I2C::createChar.
static uint8_t glyphRows[8];

Display::Display(uint8_t address, uint8_t cols, uint8_t rows)
    : lcd(address, cols, rows), cols(cols), rows(rows) {}

void Display::init() { lcd.begin(cols, rows); lcd.backlight(); }

void Display::print(const String& firstString, const String& secondString) {
  mapper.mapToCyril(firstString, lineBufFirst, LINE_BUF + 1);
  mapper.mapToCyril(secondString, lineBufSecond, LINE_BUF + 1);

  print(lineBufFirst, lineBufSecond);
}

void Display::print(const uint8_t* firstByteArray, const uint8_t* secondByteArray) {
  lcd.clear();
  writeLine(firstByteArray, 0);
  writeLine(secondByteArray, 1);
}

void Display::writeLine(const uint8_t* text, uint8_t row) {
  if (text == nullptr) {
    return;
  }

  lcd.setCursor(0, row);

  for (uint8_t i = 0; i < cols && text[i] != 0; i++) {
    uint8_t b = text[i];
    if (b >= CyrillicMapper::CYRILLIC_BASE) {
      CyrillicMapper::readGlyph(b - CyrillicMapper::CYRILLIC_BASE, glyphRows);
      lcd.createChar(0, glyphRows);
      // createChar оставляет указатель HD44780 в CGRAM;
      // перед записью символа нужно установить адрес DDRAM
      lcd.setCursor(i, row);
      lcd.write(0x00);
    } else {
      lcd.write(b);
    }
  }
}
