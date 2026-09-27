#include <Arduino.h>
#include "test_device_logic.h"
// #include "test_ir_sensor.h"
#include "change_ir_sensor_address.h"
// #include "test_temperature_control.h"
#include "date_time_test.h"

// TestDeviceLogic device_logic;

// TestIRSensor test_ir_sensor;
ChangeIRSensorAddress change_ir_sensor_address;
// TestTemperatureControl test_temperature_control;
DateTimeTest date_time_test;

void setup() {
  // device_logic.init();
  // change_ir_sensor_address.init();
  // test_ir_sensor.init();
  // test_temperature_control.init();
  date_time_test.init();
}

void loop() {
  // device_logic.loop();
  // change_ir_sensor_address.loop();
  // test_ir_sensor.loop();
  // test_temperature_control.loop();
  date_time_test.loop();
}
