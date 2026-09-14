#include <Arduino.h>
// #include "test_device_logic.h"
// #include "test_ir_sensor.h"
#include "test_bms_logic.h"

// TestDeviceLogic device_logic;

// TestIRSensor test_ir_sensor;
TestBmsLogic test_bms_logic;

void setup() {
  // device_logic.init();
  // test_ir_sensor.init();
  test_bms_logic.init();
}

void loop() {
  // device_logic.loop();
  // test_ir_sensor.loop();
  test_bms_logic.loop();
}

К ардуино подключены новый ir-сенсор, имеющий по-умолчанию стандартный адрес. 
Нужно пометь дефолтный адрес, чтобы можно было подключать сразу несколько ir- датчиков. 
Добавь ChangeIRSensorAddress : DeviceLogic, в котором есть метод writeIrSensorAddress, 
который меняет адрес ir- сенсора (тип сенсора определен в проекте). 
Адрес задается константной строкой. В методе setup() должно происходить сканирование 
всех адресов и подключение к датчику. затем , если подключение успешно- 
смена адреса датчика и повторное подключение. При успешном подключении устанавливаем 
флаг isAddressChanged. Если адрес изменен, 
пишем на 1 строке дисплея "addr chng ok", иначе- "adr chn er" .  
Используй новый класс TestIRSensor в main