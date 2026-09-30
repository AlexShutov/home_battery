// Тесты драйвера Daly BMS UART (lib/daly-bms-uart) на макете HardwareSerial.
//
// Покрытие:
//
//   | Группа              | Случаи                                                       |
//   |---------------------|--------------------------------------------------------------|
//   | Init                | настройка порта 9600 8N1                                     |
//   | кадр запроса        | компоновка A5 40 cmd 08 ... и контрольная сумма              |
//   | 0x90                | напряжение/ток/SOC, смещение тока 30000, отрицательный ток   |
//   | 0x91                | макс/мин напряжение банки, номера банок, разброс            |
//   | 0x92                | температуры со смещением 40, среднее, отрицательные values  |
//   | 0x93                | статусы 0/1/2, неизвестный статус, FET-ы, heartbeat, ёмкость|
//   | 0x94                | число банок/датчиков, зарядник, нагрузка, биты dIO, циклы   |
//   | 0x95                | 1/16/17 банок, многокадровость, обрыв кадра, invalid count  |
//   | 0x96                | температуры банок, границы числа датчиков                   |
//   | 0x97                | карта балансировки, хвост массива, целостность памяти alarm |
//   | 0x98                | раскладка битов тревог по байтам 0x00–0x06                   |
//   | update()            | полный цикл 16S (последовательность команд), обрыв на первой|
//   | 0xD9/0xDA/0x00      | установка MOS, сброс буфера, отсутствие ответа              |
//   | приём               | короткий кадр, битая контрольная сумма, значения не трогаются|

#include "Arduino.h"

#include <unity.h>

#include "daly-bms-uart.h"

// LDF не сканирует файлы из test/, поэтому библиотека не попадает в сборку
// автоматически: исходник драйвера компилируется прямо в тестовое
// приложение (путь до lib/ задаётся флагом -I в [env:native]).
// Заглушка Arduino.h из папки теста подключается первой — все вызовы
// драйвера уходят в макет HardwareSerial.
#include "daly-bms-uart.cpp"

// Макет UART и тестируемый драйвер — статические объекты (правило «Память»).
static HardwareSerial bmsTestSerial;
static Daly_BMS_UART bmsUnderTest(bmsTestSerial);

// Статический буфер сборки кадра ответа (правило «Память»).
static uint8_t frameBuf[STUB_FRAME_LENGTH];

// Сборка кадра ответа БМС: A5 01 <cmd> 08 <8 байт данных> <контрольная сумма>.
// Незаполненные байты данных — нули (как шлёт реальная БМС).
static void buildFrame(uint8_t cmd,
                       uint8_t d0 = 0, uint8_t d1 = 0, uint8_t d2 = 0, uint8_t d3 = 0,
                       uint8_t d4 = 0, uint8_t d5 = 0, uint8_t d6 = 0, uint8_t d7 = 0)
{
    frameBuf[0] = 0xA5;
    frameBuf[1] = 0x01;
    frameBuf[2] = cmd;
    frameBuf[3] = 0x08;
    frameBuf[4] = d0;
    frameBuf[5] = d1;
    frameBuf[6] = d2;
    frameBuf[7] = d3;
    frameBuf[8] = d4;
    frameBuf[9] = d5;
    frameBuf[10] = d6;
    frameBuf[11] = d7;
    uint8_t sum = 0;
    for (size_t i = 0; i < 12; i++)
    {
        sum += frameBuf[i];
    }
    frameBuf[12] = sum;
}

// Постановка собранного кадра в очередь ответов макета.
static void queueFrame() { bmsTestSerial.queueResponse(frameBuf, STUB_FRAME_LENGTH); }

// Планирование полного сеанса 16S-батареи (4 датчика температуры):
// все ответы на update() — 0x90..0x94, шесть кадров 0x95, 0x96, 0x97, 0x98.
static void queueFullBatterySession()
{
    buildFrame(0x90, 0x02, 0x14, 0, 0, 0x75, 0x62, 0x03, 0xD9); // 53.2 В, +5.0 А, 98.5 %
    queueFrame();
    buildFrame(0x91, 0x0D, 0x16, 7, 0x0C, 0xEE, 12); // 3350/3310 мВ, банки 7/12
    queueFrame();
    buildFrame(0x92, 65, 0, 46); // 25 °C / 6 °C
    queueFrame();
    buildFrame(0x93, 1, 1, 0, 0x4D, 0x00, 0x01, 0x86, 0xA0); // заряд, FET-ы, 100000 мАч
    queueFrame();
    buildFrame(0x94, 16, 4, 1, 0, 0xA5, 0x01, 0xFF); // 16S, 4 датчика, 511 циклов
    queueFrame();
    for (uint8_t f = 0; f < 5; f++)
    {
        uint16_t mv0 = (uint16_t)(3200 + 3 * (int)f);
        buildFrame(0x95, f,
                   (uint8_t)(mv0 >> 8), (uint8_t)mv0,
                   (uint8_t)((mv0 + 1) >> 8), (uint8_t)(mv0 + 1),
                   (uint8_t)((mv0 + 2) >> 8), (uint8_t)(mv0 + 2));
        queueFrame();
    }
    buildFrame(0x95, 5, 0x0C, 0x8F); // шестой кадр: только банка 16 (3215 мВ)
    queueFrame();
    buildFrame(0x96, 0, 60, 55, 50, 45); // 20/15/10/5 °C
    queueFrame();
    buildFrame(0x97, 0x01, 0x80); // балансируются банки 1 и 16
    queueFrame();
    buildFrame(0x98, 0x01, 0, 0, 0, 0, 0, 0); // одна тревога уровня 1
    queueFrame();
}

void setUp()
{
    bmsTestSerial.reset();
    bmsUnderTest.Init();
}

void tearDown() {}

// Init() настраивает порт на 9600 бод, кадр 8N1 — по спецификации Daly.
void test_init_configures_serial_9600_8n1()
{
    // setUp() уже вызвал Init(): проверяем зафиксированный вызов begin().
    TEST_ASSERT_TRUE(bmsTestSerial.isBegun());
    TEST_ASSERT_EQUAL_UINT32(9600, bmsTestSerial.begunBaud());
    TEST_ASSERT_EQUAL_UINT8(SERIAL_8N1, bmsTestSerial.begunConfig());
}

// Кадр запроса: A5 40 <cmd> 08, нулевые данные и корректная контрольная сумма.
void test_command_frame_layout_and_checksum()
{
    buildFrame(0x90, 0x02, 0x14, 0, 0, 0x75, 0x62, 0x03, 0xD9);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getPackMeasurements());

    TEST_ASSERT_EQUAL_UINT32(1, bmsTestSerial.txFrameCount());
    TEST_ASSERT_EQUAL_UINT8(0xA5, bmsTestSerial.txByte(0, 0));
    TEST_ASSERT_EQUAL_UINT8(0x40, bmsTestSerial.txByte(0, 1));
    TEST_ASSERT_EQUAL_UINT8(0x90, bmsTestSerial.txByte(0, 2));
    TEST_ASSERT_EQUAL_UINT8(0x08, bmsTestSerial.txByte(0, 3));
    for (size_t i = 4; i < 12; i++)
    {
        TEST_ASSERT_EQUAL_UINT8(0x00, bmsTestSerial.txByte(0, i));
    }
    // Сумма A5+40+90+08 = 0x17D, контрольный байт — младшие 8 бит.
    TEST_ASSERT_EQUAL_UINT8(0x7D, bmsTestSerial.txByte(0, 12));
}

// 0x90: напряжение 532*0.1 В, ток (30050-30000)*0.1 А, SOC 985*0.1 %.
void test_pack_measurements_parses_values()
{
    buildFrame(0x90, 0x02, 0x14, 0, 0, 0x75, 0x62, 0x03, 0xD9);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getPackMeasurements());
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 53.2f, bmsUnderTest.get.packVoltage);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 5.0f, bmsUnderTest.get.packCurrent);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 98.5f, bmsUnderTest.get.packSOC);
}

// 0x90: смещение тока 30000 — нулевой и отрицательный (разряд) ток.
void test_pack_current_offset_30000()
{
    buildFrame(0x90, 0x02, 0x14, 0, 0, 0x75, 0x30, 0x03, 0xE8); // 30000 -> 0 А
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getPackMeasurements());
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 0.0f, bmsUnderTest.get.packCurrent);

    buildFrame(0x90, 0x02, 0x14, 0, 0, 0x74, 0x68, 0x03, 0xE8); // 29800 -> -20 А
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getPackMeasurements());
    TEST_ASSERT_FLOAT_WITHIN(0.05f, -20.0f, bmsUnderTest.get.packCurrent);
}

// Короткий кадр (обрыв связи) — ошибка приёма, метод возвращает false.
void test_receive_wrong_length_fails()
{
    buildFrame(0x90);
    bmsTestSerial.queueResponse(frameBuf, 10); // только 10 байт из 13

    TEST_ASSERT_FALSE(bmsUnderTest.getPackMeasurements());
}

// Битая контрольная сумма — кадр отбраковывается.
void test_receive_checksum_error_fails()
{
    buildFrame(0x90);
    frameBuf[12] ^= 0xFF;
    queueFrame();

    TEST_ASSERT_FALSE(bmsUnderTest.getPackMeasurements());
}

// Неудачный приём не затирает прошлые показания (телеметрия остаётся с
// последнего успешного чтения — на неё ориентируется экран станции).
void test_failed_receive_keeps_previous_values()
{
    bmsUnderTest.get.packVoltage = -1.0f;

    buildFrame(0x90);
    frameBuf[12] ^= 0xFF;
    queueFrame();

    TEST_ASSERT_FALSE(bmsUnderTest.getPackMeasurements());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, bmsUnderTest.get.packVoltage);
}

// 0x91: макс/мин напряжение банка в мВ, номера банок и разброс.
void test_min_max_cell_voltage_parses()
{
    buildFrame(0x91, 0x0D, 0x16, 7, 0x0C, 0xEE, 12);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getMinMaxCellVoltage());
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 3350.0f, bmsUnderTest.get.maxCellmV);
    TEST_ASSERT_EQUAL_INT(7, bmsUnderTest.get.maxCellVNum);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 3310.0f, bmsUnderTest.get.minCellmV);
    TEST_ASSERT_EQUAL_INT(12, bmsUnderTest.get.minCellVNum);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 40.0f, bmsUnderTest.get.cellDiff);
}

// 0x92: температуры приходят со смещением +40; среднее — целочисленное.
void test_pack_temp_with_forty_offset()
{
    buildFrame(0x92, 65, 0, 46);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getPackTemp());
    TEST_ASSERT_EQUAL_INT(25, bmsUnderTest.get.tempMax);
    TEST_ASSERT_EQUAL_INT(6, bmsUnderTest.get.tempMin);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 15.0f, bmsUnderTest.get.tempAverage);
}

// 0x92: отрицательные температуры (байт меньше смещения 40).
void test_pack_temp_negative()
{
    buildFrame(0x92, 20, 0, 10);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getPackTemp());
    TEST_ASSERT_EQUAL_INT(-20, bmsUnderTest.get.tempMax);
    TEST_ASSERT_EQUAL_INT(-30, bmsUnderTest.get.tempMin);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -25.0f, bmsUnderTest.get.tempAverage);
}

// 0x93: коды статуса 0/1/2 и четырёхбайтовая остаточная ёмкость.
void test_mos_status_parse()
{
    buildFrame(0x93, 0, 1, 0, 0x4D, 0x00, 0x01, 0x86, 0xA0);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getDischargeChargeMosStatus());
    TEST_ASSERT_EQUAL_UINT8(0, bmsUnderTest.get.chargeDischargeStatus);

    buildFrame(0x93, 1, 1, 0, 0x4D, 0x00, 0x01, 0x86, 0xA0);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getDischargeChargeMosStatus());
    TEST_ASSERT_EQUAL_UINT8(1, bmsUnderTest.get.chargeDischargeStatus);

    buildFrame(0x93, 2, 1, 0, 0x4D, 0x00, 0x01, 0x86, 0xA0);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getDischargeChargeMosStatus());
    TEST_ASSERT_EQUAL_UINT8(2, bmsUnderTest.get.chargeDischargeStatus);

    TEST_ASSERT_TRUE(bmsUnderTest.get.chargeFetState);
    TEST_ASSERT_FALSE(bmsUnderTest.get.disChargeFetState);
    TEST_ASSERT_EQUAL_INT(0x4D, bmsUnderTest.get.bmsHeartBeat);
    TEST_ASSERT_EQUAL_UINT32(100000, bmsUnderTest.get.resCapacitymAh);
}

// 0x93: неизвестный код статуса не меняет прошлый (древний switch без default).
void test_mos_status_unknown_code_keeps_previous()
{
    buildFrame(0x93, 2);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getDischargeChargeMosStatus());
    TEST_ASSERT_EQUAL_UINT8(2, bmsUnderTest.get.chargeDischargeStatus);

    buildFrame(0x93, 7); // мусорный байт статуса
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getDischargeChargeMosStatus());
    TEST_ASSERT_EQUAL_UINT8(2, bmsUnderTest.get.chargeDischargeStatus);
}

// 0x94: число банок/датчиков, зарядник, нагрузка, биты dIO и число циклов.
void test_status_info_parses()
{
    buildFrame(0x94, 16, 4, 1, 0, 0xA5, 0x01, 0xFF);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getStatusInfo());
    TEST_ASSERT_EQUAL_INT(16, bmsUnderTest.get.numberOfCells);
    TEST_ASSERT_EQUAL_INT(4, bmsUnderTest.get.numOfTempSensors);
    TEST_ASSERT_TRUE(bmsUnderTest.get.chargeState);
    TEST_ASSERT_FALSE(bmsUnderTest.get.loadState);
    // 0xA5 = биты 0, 2, 5, 7.
    TEST_ASSERT_TRUE(bmsUnderTest.get.dIO[0]);
    TEST_ASSERT_FALSE(bmsUnderTest.get.dIO[1]);
    TEST_ASSERT_TRUE(bmsUnderTest.get.dIO[2]);
    TEST_ASSERT_FALSE(bmsUnderTest.get.dIO[3]);
    TEST_ASSERT_FALSE(bmsUnderTest.get.dIO[4]);
    TEST_ASSERT_TRUE(bmsUnderTest.get.dIO[5]);
    TEST_ASSERT_FALSE(bmsUnderTest.get.dIO[6]);
    TEST_ASSERT_TRUE(bmsUnderTest.get.dIO[7]);
    TEST_ASSERT_EQUAL_INT(511, bmsUnderTest.get.bmsCycles);
}

// 0x95: одна банка — один кадр, лишние слоты кадра не читаются.
void test_cell_voltages_single_cell()
{
    bmsUnderTest.get.numberOfCells = 1;
    bmsUnderTest.get.cellVmV[1] = -1.0f; // сторож: слот за пределами банок

    buildFrame(0x95, 0, 0x0C, 0x80, 0x0C, 0x81, 0x0C, 0x82);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getCellVoltages());
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 3200.0f, bmsUnderTest.get.cellVmV[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, bmsUnderTest.get.cellVmV[1]);
}

// 0x95: 16 банок раскладываются по 6 кадрам (в последнем — одна банка).
void test_cell_voltages_16_cells_multiframe()
{
    bmsUnderTest.get.numberOfCells = 16;

    for (uint8_t f = 0; f < 5; f++)
    {
        uint16_t mv0 = (uint16_t)(3300 + 3 * (int)f);
        buildFrame(0x95, f,
                   (uint8_t)(mv0 >> 8), (uint8_t)mv0,
                   (uint8_t)((mv0 + 1) >> 8), (uint8_t)(mv0 + 1),
                   (uint8_t)((mv0 + 2) >> 8), (uint8_t)(mv0 + 2));
        queueFrame();
    }
    buildFrame(0x95, 5, 0x0C, 0xF3); // шестой кадр: только банка 16 (3315 мВ)
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getCellVoltages());
    for (int c = 0; c < 16; c++)
    {
        TEST_ASSERT_FLOAT_WITHIN(0.5f, (float)(3300 + c), bmsUnderTest.get.cellVmV[c]);
    }
}

// 0x95: верхняя граница 17 банок (массив как раз на 17) — тоже допустима.
void test_cell_voltages_17_cells_boundary()
{
    bmsUnderTest.get.numberOfCells = 17;

    for (uint8_t f = 0; f < 5; f++)
    {
        buildFrame(0x95, f, 0x0C, 0x80, 0x0C, 0x80, 0x0C, 0x80);
        queueFrame();
    }
    buildFrame(0x95, 5, 0x0C, 0x80, 0x0C, 0x81); // банки 16 и 17
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getCellVoltages());
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 3201.0f, bmsUnderTest.get.cellVmV[16]);
}

// 0x95: недопустимое число банок — отказ без отправки команды.
void test_cell_voltages_invalid_count_rejected()
{
    bmsUnderTest.get.numberOfCells = 0;
    TEST_ASSERT_FALSE(bmsUnderTest.getCellVoltages());
    TEST_ASSERT_EQUAL_UINT32(0, bmsTestSerial.txFrameCount());

    bmsUnderTest.get.numberOfCells = 18; // больше MAX_NUMBER_CELLS
    TEST_ASSERT_FALSE(bmsUnderTest.getCellVoltages());
    TEST_ASSERT_EQUAL_UINT32(0, bmsTestSerial.txFrameCount());
}

// 0x95: обрыв связи на промежуточном кадре — метод возвращает false.
void test_cell_voltages_missing_frame_fails()
{
    bmsUnderTest.get.numberOfCells = 6; // нужно 2 кадра

    buildFrame(0x95, 0, 0x0C, 0x80, 0x0C, 0x80, 0x0C, 0x80);
    queueFrame();
    // второй кадр не планируем — БМС «замолчала»

    TEST_ASSERT_FALSE(bmsUnderTest.getCellVoltages());
}

// 0x96: температуры банок со смещением +40, по 7 штук в кадре.
void test_cell_temperature_parses()
{
    bmsUnderTest.get.numOfTempSensors = 4;

    buildFrame(0x96, 0, 60, 55, 50, 45);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getCellTemperature());
    TEST_ASSERT_EQUAL_INT(20, bmsUnderTest.get.cellTemperature[0]);
    TEST_ASSERT_EQUAL_INT(15, bmsUnderTest.get.cellTemperature[1]);
    TEST_ASSERT_EQUAL_INT(10, bmsUnderTest.get.cellTemperature[2]);
    TEST_ASSERT_EQUAL_INT(5, bmsUnderTest.get.cellTemperature[3]);
}

// 0x96: граница 5 датчиков — один кадр; 0 и 6 датчиков — отказ без запроса.
void test_cell_temperature_count_boundaries()
{
    bmsUnderTest.get.numOfTempSensors = 5;
    buildFrame(0x96, 0, 60, 55, 50, 45, 40);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getCellTemperature());
    TEST_ASSERT_EQUAL_INT(0, bmsUnderTest.get.cellTemperature[4]);

    bmsUnderTest.get.numOfTempSensors = 0;
    TEST_ASSERT_FALSE(bmsUnderTest.getCellTemperature());

    bmsUnderTest.get.numOfTempSensors = 6;
    TEST_ASSERT_FALSE(bmsUnderTest.getCellTemperature());
    // Успешный обмен был только для 5 датчиков.
    TEST_ASSERT_EQUAL_UINT32(1, bmsTestSerial.txFrameCount());
}

// 0x97: бит 0 байта 0 — банка 1, бит 7 байта 1 — банка 16.
void test_balance_state_maps_bits_to_cells()
{
    bmsUnderTest.get.numberOfCells = 16;

    buildFrame(0x97, 0x01, 0x80);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getCellBalanceState());
    TEST_ASSERT_TRUE(bmsUnderTest.get.cellBalanceState[0]);
    TEST_ASSERT_TRUE(bmsUnderTest.get.cellBalanceState[15]);
    for (int c = 1; c < 15; c++)
    {
        TEST_ASSERT_FALSE(bmsUnderTest.get.cellBalanceState[c]);
    }
    TEST_ASSERT_TRUE(bmsUnderTest.get.cellBalanceActive);
}

// 0x97: пустая карта — балансировка не активна, хвост массива очищен.
void test_balance_state_inactive_when_clear()
{
    bmsUnderTest.get.numberOfCells = 16;
    bmsUnderTest.get.cellBalanceState[16] = true; // мусор за пределами банок

    buildFrame(0x97, 0x00, 0x00);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getCellBalanceState());
    TEST_ASSERT_FALSE(bmsUnderTest.get.cellBalanceActive);
    for (int c = 0; c < 16; c++)
    {
        TEST_ASSERT_FALSE(bmsUnderTest.get.cellBalanceState[c]);
    }
    TEST_ASSERT_FALSE(bmsUnderTest.get.cellBalanceState[16]);
}

// 0x97, регрессия критичного бага: прежний цикл разбирал все 47 битов карты
// и писал их в массив на MAX_NUMBER_CELLS=17 элементов — выход за границы
// затирал cellBalanceActive и поля структуры alarm (ложные тревоги или
// пропажа настоящих после 0x97). Тест: карта из одних единиц не должна
// задевать тревоги, установленные предыдущей командой 0x98.
void test_balance_state_does_not_touch_neighbor_memory()
{
    buildFrame(0x94, 16, 4);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getStatusInfo());

    buildFrame(0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getFailureCodes());
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelOneCellVoltageTooHigh);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.failureOfLowVoltageNoCharging);

    buildFrame(0x97, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getCellBalanceState());

    // Все биты карты установлены, но тревоги обязаны остаться снятыми.
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelOneCellVoltageTooHigh);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelTwoCellVoltageTooHigh);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelOneCellVoltageTooLow);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelTwoDischargeTempTooLow);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelOneChargeCurrentTooHigh);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelTwoTempSensorDifferenceTooHigh);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.failureOfChargeFETAdhesion);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.failureOfAFEAcquisitionModule);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.failureOfPrechargeModule);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.failureOfLowVoltageNoCharging);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.failureOfShortCircuitProtection);

    // Сами банки при этом размечены корректно.
    for (int c = 0; c < 16; c++)
    {
        TEST_ASSERT_TRUE(bmsUnderTest.get.cellBalanceState[c]);
    }
    TEST_ASSERT_TRUE(bmsUnderTest.get.cellBalanceActive);
}

// 0x98: раскладка битов тревог по байтам 0x00–0x06 (по одному паттерну на байт).
void test_failure_codes_bit_mapping()
{
    buildFrame(0x98, 0x01, 0x80, 0x55, 0x08, 0x10, 0x20, 0x0F);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.getFailureCodes());

    // Байт 0x00: бит 0 — уровень 1 высокого напряжения банки.
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.levelOneCellVoltageTooHigh);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelTwoCellVoltageTooHigh);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelOneCellVoltageTooLow);
    // Байт 0x01: бит 7 — уровень 2 низкой температуры разряда.
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelOneChargeTempTooHigh);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.levelTwoDischargeTempTooLow);
    // Байт 0x02: 0x55 — чётные биты (уровень 1 токов и SOC).
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.levelOneChargeCurrentTooHigh);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelTwoChargeCurrentTooHigh);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.levelOneDischargeCurrentTooHigh);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.levelOneStateOfChargeTooHigh);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.levelOneStateOfChargeTooLow);
    // Байт 0x03: бит 3 — уровень 2 разброса температур датчиков.
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.levelOneCellVoltageDifferenceTooHigh);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.levelTwoTempSensorDifferenceTooHigh);
    // Байт 0x04: бит 4 — залипание зарядного FET.
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.failureOfChargeFETAdhesion);
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.failureOfDischargeFETAdhesion);
    // Байт 0x05: бит 5 — отказ модуля предзаряда.
    TEST_ASSERT_FALSE(bmsUnderTest.alarm.failureOfEEPROMStorageModule);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.failureOfPrechargeModule);
    // Байт 0x06: 0x0F — все четыре отказа.
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.failureOfCurrentSensorModule);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.failureOfMainVoltageSensorModule);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.failureOfShortCircuitProtection);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.failureOfLowVoltageNoCharging);
}

// update(): полный цикл опроса 16S-батареи — все команды в правильном
// порядке и согласованные данные в структурах get/alarm.
void test_update_full_cycle()
{
    queueFullBatterySession();

    TEST_ASSERT_TRUE(bmsUnderTest.update());

    // Последовательность запросов: 0x90–0x94, ОДИН запрос 0x95 (БМС отвечает
    // серией из шести кадров), затем 0x96–0x98.
    TEST_ASSERT_EQUAL_UINT32(9, bmsTestSerial.txFrameCount());
    TEST_ASSERT_EQUAL_UINT8(0x90, bmsTestSerial.txByte(0, 2));
    TEST_ASSERT_EQUAL_UINT8(0x91, bmsTestSerial.txByte(1, 2));
    TEST_ASSERT_EQUAL_UINT8(0x92, bmsTestSerial.txByte(2, 2));
    TEST_ASSERT_EQUAL_UINT8(0x93, bmsTestSerial.txByte(3, 2));
    TEST_ASSERT_EQUAL_UINT8(0x94, bmsTestSerial.txByte(4, 2));
    TEST_ASSERT_EQUAL_UINT8(0x95, bmsTestSerial.txByte(5, 2));
    TEST_ASSERT_EQUAL_UINT8(0x96, bmsTestSerial.txByte(6, 2));
    TEST_ASSERT_EQUAL_UINT8(0x97, bmsTestSerial.txByte(7, 2));
    TEST_ASSERT_EQUAL_UINT8(0x98, bmsTestSerial.txByte(8, 2));

    // Сводные значения сеанса.
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 53.2f, bmsUnderTest.get.packVoltage);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 5.0f, bmsUnderTest.get.packCurrent);
    TEST_ASSERT_EQUAL_INT(25, bmsUnderTest.get.tempMax);
    TEST_ASSERT_EQUAL_INT(16, bmsUnderTest.get.numberOfCells);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 3215.0f, bmsUnderTest.get.cellVmV[15]);
    TEST_ASSERT_EQUAL_INT(20, bmsUnderTest.get.cellTemperature[0]);
    TEST_ASSERT_TRUE(bmsUnderTest.get.cellBalanceActive);
    TEST_ASSERT_TRUE(bmsUnderTest.get.chargeFetState);
    TEST_ASSERT_EQUAL_UINT32(100000, bmsUnderTest.get.resCapacitymAh);
    TEST_ASSERT_TRUE(bmsUnderTest.alarm.levelOneCellVoltageTooHigh);
}

// update(): первый же битый ответ прерывает цикл — дальше запросов нет.
void test_update_aborts_on_first_failed_command()
{
    buildFrame(0x90);
    frameBuf[12] ^= 0xFF;
    queueFrame();

    TEST_ASSERT_FALSE(bmsUnderTest.update());
    TEST_ASSERT_EQUAL_UINT32(1, bmsTestSerial.txFrameCount());
}

// 0xDA: включение ставит байт данных 0x01, выключение — 0x00; после
// обмена буфер запроса очищается (следующий запрос снова с нулём).
void test_set_charge_mos_frames()
{
    buildFrame(0xDA, 0x01);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.setChargeMOS(true));
    TEST_ASSERT_EQUAL_UINT8(0xDA, bmsTestSerial.txByte(0, 2));
    TEST_ASSERT_EQUAL_UINT8(0x01, bmsTestSerial.txByte(0, 4));

    buildFrame(0xDA, 0x00);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.setChargeMOS(false));
    TEST_ASSERT_EQUAL_UINT8(0xDA, bmsTestSerial.txByte(1, 2));
    TEST_ASSERT_EQUAL_UINT8(0x00, bmsTestSerial.txByte(1, 4));

    buildFrame(0x90);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.getPackMeasurements());
    TEST_ASSERT_EQUAL_UINT8(0x00, bmsTestSerial.txByte(2, 4));
}

// 0xD9: то же для разрядного MOS.
void test_set_discharge_mos_frames()
{
    buildFrame(0xD9, 0x01);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.setDischargeMOS(true));
    TEST_ASSERT_EQUAL_UINT8(0xD9, bmsTestSerial.txByte(0, 2));
    TEST_ASSERT_EQUAL_UINT8(0x01, bmsTestSerial.txByte(0, 4));

    buildFrame(0xD9, 0x00);
    queueFrame();
    TEST_ASSERT_TRUE(bmsUnderTest.setDischargeMOS(false));
    TEST_ASSERT_EQUAL_UINT8(0xD9, bmsTestSerial.txByte(1, 2));
    TEST_ASSERT_EQUAL_UINT8(0x00, bmsTestSerial.txByte(1, 4));
}

// Команды MOS без ответа БМС возвращают false.
void test_set_mos_fails_without_response()
{
    TEST_ASSERT_FALSE(bmsUnderTest.setChargeMOS(true));
    TEST_ASSERT_FALSE(bmsUnderTest.setDischargeMOS(false));
    TEST_ASSERT_FALSE(bmsUnderTest.setBmsReset());
}

// 0x00: команда сброса БМС с корректной контрольной суммой кадра.
void test_bms_reset_frame()
{
    buildFrame(0x00);
    queueFrame();

    TEST_ASSERT_TRUE(bmsUnderTest.setBmsReset());
    TEST_ASSERT_EQUAL_UINT8(0x00, bmsTestSerial.txByte(0, 2));
    // A5+40+00+08 = 0xED.
    TEST_ASSERT_EQUAL_UINT8(0xED, bmsTestSerial.txByte(0, 12));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_init_configures_serial_9600_8n1);
    RUN_TEST(test_command_frame_layout_and_checksum);
    RUN_TEST(test_pack_measurements_parses_values);
    RUN_TEST(test_pack_current_offset_30000);
    RUN_TEST(test_receive_wrong_length_fails);
    RUN_TEST(test_receive_checksum_error_fails);
    RUN_TEST(test_failed_receive_keeps_previous_values);
    RUN_TEST(test_min_max_cell_voltage_parses);
    RUN_TEST(test_pack_temp_with_forty_offset);
    RUN_TEST(test_pack_temp_negative);
    RUN_TEST(test_mos_status_parse);
    RUN_TEST(test_mos_status_unknown_code_keeps_previous);
    RUN_TEST(test_status_info_parses);
    RUN_TEST(test_cell_voltages_single_cell);
    RUN_TEST(test_cell_voltages_16_cells_multiframe);
    RUN_TEST(test_cell_voltages_17_cells_boundary);
    RUN_TEST(test_cell_voltages_invalid_count_rejected);
    RUN_TEST(test_cell_voltages_missing_frame_fails);
    RUN_TEST(test_cell_temperature_parses);
    RUN_TEST(test_cell_temperature_count_boundaries);
    RUN_TEST(test_balance_state_maps_bits_to_cells);
    RUN_TEST(test_balance_state_inactive_when_clear);
    RUN_TEST(test_balance_state_does_not_touch_neighbor_memory);
    RUN_TEST(test_failure_codes_bit_mapping);
    RUN_TEST(test_update_full_cycle);
    RUN_TEST(test_update_aborts_on_first_failed_command);
    RUN_TEST(test_set_charge_mos_frames);
    RUN_TEST(test_set_discharge_mos_frames);
    RUN_TEST(test_set_mos_fails_without_response);
    RUN_TEST(test_bms_reset_frame);
    return UNITY_END();
}
