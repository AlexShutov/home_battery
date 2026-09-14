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