#include "test_device_logic.h"

TestDeviceLogic device_logic;

void setup() {
  device_logic.init();
}

void loop() {
  device_logic.loop();
}
