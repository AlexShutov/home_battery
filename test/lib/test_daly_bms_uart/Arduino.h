// Заглушка Arduino.h для нативных юнит-тестов драйвера Daly BMS
// (lib/daly-bms-uart). Определяет только то, что использует драйвер:
// константу SERIAL_8N1, макрос bitRead и скриптуемый макет HardwareSerial.
// Файл лежит в папке тестового приложения: PlatformIO добавляет её в
// пути включения, и #include "Arduino.h" из daly-bms-uart.cpp находит
// эту заглушку вместо каркаса Arduino.
#ifndef ARDUINO_H_NATIVE_STUB
#define ARDUINO_H_NATIVE_STUB

#include <stdint.h>
#include <string.h>

// Конфигурация кадра 8N1 (значение произвольно: драйвер только передаёт его).
#define SERIAL_8N1 0x06

// Чтение бита числа — как макрос Arduino (используется драйвером).
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

// Длина кадра протокола Daly.
#define STUB_FRAME_LENGTH 13
// Ёмкость журнала запросов и очереди ответов: полный цикл update() шлёт 13 команд.
#define STUB_TX_LOG_FRAMES 16
#define STUB_PENDING_FRAMES 20
// FIFO приёма: на один запрос в шину приходит не больше одного кадра.
#define STUB_RX_FIFO_SIZE 32

// Макет аппаратного UART. Ответы БМС планируются заранее (queueResponse) и
// «приходят» в FIFO приёма только после записи запроса — как на реальной
// шине, где ответ следует за командой (иначе предзапланированные байты
// съедал бы дренаж приёма в sendCommand). Запросы попадают в журнал,
// чтобы тесты проверяли байты отправленных кадров.
class HardwareSerial
{
public:
    HardwareSerial() { reset(); }

    // Сброс состояния макета между тестами.
    void reset()
    {
        rxFill = 0;
        rxRead = 0;
        pendingCount = 0;
        txCount = 0;
        begun = false;
        beginBaud = 0;
        beginConfig = 0;
    }

    // Настройка порта: драйвер вызывает begin(9600, SERIAL_8N1).
    void begin(unsigned long baud, uint8_t config)
    {
        begun = true;
        beginBaud = baud;
        beginConfig = config;
    }

    // Чтение байта из FIFO приёма; -1 — данных нет (как в Arduino).
    // Когда буфер опустошён, индексы возвращаются в начало — ёмкости
    // хватает на любой сеанс из нескольких обменов подряд.
    int read()
    {
        if (rxRead >= rxFill)
        {
            rxRead = 0;
            rxFill = 0;
            return -1;
        }
        return rxFifo[rxRead++];
    }

    // Запись запроса: кадр попадает в журнал. Ответы доставляются не здесь,
    // а в readBytes по мере нехватки данных — так моделируется шина, где БМС
    // шлёт кадры вслед за запросом, в том числе сериями (шесть кадров 0x95).
    size_t write(const uint8_t *buffer, size_t size)
    {
        if (txCount < STUB_TX_LOG_FRAMES)
        {
            size_t n = (size < STUB_FRAME_LENGTH) ? size : STUB_FRAME_LENGTH;
            memcpy(txLog[txCount], buffer, n);
            txSizes[txCount] = n;
            txCount++;
        }
        return size;
    }

    // Неблокирующий аналог readBytes: отдаёт не больше, чем реально есть
    // (реальный readBytes ждёт до таймаута — тесту достаточно короткого
    // чтения, чтобы имитировать обрыв связи). Если данные кончились, а
    // очередь ответов нет — доставляет следующий запланированный кадр.
    size_t readBytes(uint8_t *buffer, size_t length)
    {
        size_t got = 0;
        while (got < length)
        {
            int b = read();
            if (b < 0)
            {
                if (!deliverPending())
                {
                    break;
                }
                continue;
            }
            buffer[got] = (uint8_t)b;
            got++;
        }
        return got;
    }

    // Планирование ответа БМС на следующий по счёту запрос.
    void queueResponse(const uint8_t *frameBytes, size_t size)
    {
        if (pendingCount >= STUB_PENDING_FRAMES)
        {
            return;
        }
        size_t n = (size < STUB_FRAME_LENGTH) ? size : STUB_FRAME_LENGTH;
        memcpy(pendingFrames[pendingCount], frameBytes, n);
        pendingSizes[pendingCount] = n;
        pendingCount++;
    }

    // Число записанных драйвером запросов.
    size_t txFrameCount() const { return txCount; }

    // Байт журнала запросов по индексам кадра и байта в нём.
    uint8_t txByte(size_t frameIdx, size_t byteIdx) const { return txLog[frameIdx][byteIdx]; }

    // Параметры вызова begin() для проверок Init().
    bool isBegun() const { return begun; }
    unsigned long begunBaud() const { return beginBaud; }
    uint8_t begunConfig() const { return beginConfig; }

private:
    // Выкладка следующего запланированного кадра в FIFO приёма.
    bool deliverPending()
    {
        if (pendingCount == 0)
        {
            return false;
        }
        for (size_t i = 0; i < pendingSizes[0]; i++)
        {
            pushRxByte(pendingFrames[0][i]);
        }
        for (size_t f = 1; f < pendingCount; f++)
        {
            memcpy(pendingFrames[f - 1], pendingFrames[f], pendingSizes[f]);
            pendingSizes[f - 1] = pendingSizes[f];
        }
        pendingCount--;
        return true;
    }

    // Выкладка байта ответа в FIFO приёма.
    void pushRxByte(uint8_t b)
    {
        if (rxFill < STUB_RX_FIFO_SIZE)
        {
            rxFifo[rxFill++] = b;
        }
    }

    // FIFO приёма: линейный буфер на один доставленный кадр.
    uint8_t rxFifo[STUB_RX_FIFO_SIZE];
    size_t rxFill;
    size_t rxRead;

    // Очередь запланированных ответов (доставляются по одному на запрос).
    uint8_t pendingFrames[STUB_PENDING_FRAMES][STUB_FRAME_LENGTH];
    size_t pendingSizes[STUB_PENDING_FRAMES];
    size_t pendingCount;

    // Журнал записанных запросов.
    uint8_t txLog[STUB_TX_LOG_FRAMES][STUB_FRAME_LENGTH];
    size_t txSizes[STUB_TX_LOG_FRAMES];
    size_t txCount;

    // Зафиксированный вызов begin().
    bool begun;
    unsigned long beginBaud;
    uint8_t beginConfig;
};

#endif // ARDUINO_H_NATIVE_STUB
