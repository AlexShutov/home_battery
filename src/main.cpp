#include <Arduino.h>
#include "test_device_logic.h"
// #include "temperature/test_ir_sensor.h"
#include "temperature/change_ir_sensor_address.h"
// #include "temperature/test_temperature_control.h"
// #include "datetime/date_time_test.h"
#include "datetime/set_date_time.h"

// TestDeviceLogic device_logic;

// TestIRSensor test_ir_sensor;
ChangeIRSensorAddress change_ir_sensor_address;
// TestTemperatureControl test_temperature_control;
// DateTimeTest date_time_test;
SetDateTime set_date_time;

void setup() {
  // device_logic.init();
  // change_ir_sensor_address.init();
  // test_ir_sensor.init();
  // test_temperature_control.init();
  // date_time_test.init();
  set_date_time.init();
}

void loop() {
  // device_logic.loop();
  // change_ir_sensor_address.loop();
  // test_ir_sensor.loop();
  // test_temperature_control.loop();
  // date_time_test.loop();
  set_date_time.loop();
}
