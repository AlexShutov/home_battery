#pragma once

#include <Arduino.h>

class CyrillicMapper {
public:
    // Кириллические байты начинаются с этой границы, младшие 7 бит — индекс глифа
    static const uint8_t CYRILLIC_BASE = 0x80;

    static const uint8_t GLYPH_COUNT = 66;

    // Преобразует текст в массив байтов в буфере out (размером outSize байт,
    // буфер принадлежит вызывающей стороне). ASCII (0x00-0x7F) проходят без
    // изменений, каждый кириллический символ заменяется на
    // CYRILLIC_BASE + индекс глифа, готовый для вывода на дисплей.
    // Декодирует UTF-8; одиночные байты windows-1251 (0xC0-0xFF, 0xA2, 0xA3)
    // и верхний регистр cp866 (0x80-0x9F) также распознаются.
    // Результат завершается нулём. Возвращает количество записанных байт
    // (без учёта нуль-терминатора).
    uint8_t mapToCyril(const String& text, uint8_t* out, uint8_t outSize) const;
    uint8_t mapToCyril(const char* text, uint8_t* out, uint8_t outSize) const;
    uint8_t mapToCyril(const uint8_t* data, uint8_t length, uint8_t* out, uint8_t outSize) const;

    // Истинно, если кодовая точка принадлежит кириллическому блоку Unicode
    static bool isCyrillic(uint16_t codePoint);

    // Индекс кириллического кода во встроенной таблице глифов, 0xFF если глифа нет
    static uint8_t glyphIndex(uint16_t codePoint);

    // Копирует битовую схему глифа 5x8 (8 строк, бит 0 = левый столбец) в out[8]
    static void readGlyph(uint8_t glyphIndex_, uint8_t* out);
};
