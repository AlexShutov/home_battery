#include <Arduino.h>
// #include "test_device_logic.h"
// #include "test_ir_sensor.h"
#include "change_ir_sensor_address.h"

// TestDeviceLogic device_logic;

// TestIRSensor test_ir_sensor;
ChangeIRSensorAddress change_ir_sensor_address;

void setup() {
  // device_logic.init();
  change_ir_sensor_address.init();
}

void loop() {
  // device_logic.loop();
  change_ir_sensor_address.loop();
}