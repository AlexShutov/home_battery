#include "display.h"
#include "cyrillic_mapper.h"

const uint8_t Display::LINE_BUF;

// Экземпляр маппера в статическом хранилище (метод не зависит от состояния объекта).
static const CyrillicMapper mapper;

// Буфер битовой схемы глифа для LiquidCrystal_I2C::createChar.
static uint8_t glyphRows[8];

// Таблица CGRAM: для каждого индекса глифа хранит назначенный слот (0-7),
// либо 0xFF, если глиф не помещается в 8 доступных знакомест.
static uint8_t slotForGlyph[CyrillicMapper::GLYPH_COUNT];

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

  // HD44780 имеет только 8 пользовательских знакомест, поэтому каждому
  // уникальному глифу выделяется собственный стабильный слот (0-7).
  // Повторное переопределение уже выведенного слота заставляет дисплей
  // перерисовывать все клетки, ссылающиеся на него, одним и тем же символом.
  for (uint8_t g = 0; g < CyrillicMapper::GLYPH_COUNT; g++) {
    slotForGlyph[g] = 0xFF;
  }

  uint8_t used = 0;

  if (firstByteArray != nullptr) {
    for (uint8_t i = 0; i < cols && firstByteArray[i] != 0; i++) {
      uint8_t b = firstByteArray[i];
      if (b >= CyrillicMapper::CYRILLIC_BASE) {
        uint8_t gi = b - CyrillicMapper::CYRILLIC_BASE;
        if (slotForGlyph[gi] == 0xFF && used < 8) {
          slotForGlyph[gi] = used++;
        }
      }
    }
  }

  if (secondByteArray != nullptr) {
    for (uint8_t i = 0; i < cols && secondByteArray[i] != 0; i++) {
      uint8_t b = secondByteArray[i];
      if (b >= CyrillicMapper::CYRILLIC_BASE) {
        uint8_t gi = b - CyrillicMapper::CYRILLIC_BASE;
        if (slotForGlyph[gi] == 0xFF && used < 8) {
          slotForGlyph[gi] = used++;
        }
      }
    }
  }

  // Загружаем битовые схемы назначенных глифов в CGRAM до вывода текста.
  for (uint8_t slot = 0; slot < used; slot++) {
    for (uint8_t g = 0; g < CyrillicMapper::GLYPH_COUNT; g++) {
      if (slotForGlyph[g] == slot) {
        CyrillicMapper::readGlyph(g, glyphRows);
        lcd.createChar(slot, glyphRows);
        break;
      }
    }
  }

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
      uint8_t slot = slotForGlyph[b - CyrillicMapper::CYRILLIC_BASE];
      if (slot < 8) {
        lcd.write(slot);
      } else {
        lcd.write(' ');
      }
    } else {
      lcd.write(b);
    }
  }
}
