#include <Arduino.h>
// #include "test_device_logic.h"
#include "test_ir_sensor.h"

// TestDeviceLogic device_logic;

TestIRSensor test_ir_sensor;

void setup() {
  // device_logic.init();
  test_ir_sensor.init();
}

void loop() {
  // device_logic.loop();
  test_ir_sensor.loop();
}
