// FreeRTOS подключается первым заголовком (требование библиотеки).
#include <Arduino_FreeRTOS.h>
#include <task.h>

#include <Arduino.h>
#include "test_device_logic.h"
// #include "temperature/test_ir_sensor.h"
// #include "temperature/change_ir_sensor_address.h"
// #include "temperature/test_temperature_control.h"
// #include "datetime/date_time_test.h"
#include "datetime/set_date_time.h"
// #include "device/battery_station.h"

void loop();

// Период планирования главного цикла, мс.
static const TickType_t LOOP_PERIOD_TICKS = pdMS_TO_TICKS(500);

// Стек задачи-планировщика, байт: её единственная обязанность — проставлять
// флаг, поэтому стек минимальный.
static const uint16_t SCHEDULER_TASK_STACK_SIZE = 96;

// Флаг «пора выполнить работу главного цикла»: проставляется задачей-
// планировщиком раз в 500 мс, проверяется и сбрасывается главным циклом.
static volatile bool isPendingTask = false;

// Задача-планировщик: раз в 500 мс проставляет флаг isPendingTask.
// Вся работа приложения выполняется в главном цикле на большом стеке
// (idle-контекст), а не на стеке этой задачи.
static void schedulerTask(void* pvParameters) {
  (void)pvParameters;
  TickType_t wakeTime = xTaskGetTickCount();
  for (;;) {
    isPendingTask = true;
    vTaskDelayUntil(&wakeTime, LOOP_PERIOD_TICKS);
  }
}

// Главный цикл приложения: выполняется в idle-контексте FreeRTOS.
// Стек idle-задачи задаётся конфигом (configMINIMAL_STACK_SIZE) и достаточен
// для полноценной работы — в отличие от стека отдельной задачи.
extern "C" void vApplicationIdleHook(void) {
  if (isPendingTask) {
    isPendingTask = false;
    loop();
  }
}

// TestDeviceLogic device_logic;

// TestIRSensor test_ir_sensor;
// ChangeIRSensorAddress change_ir_sensor_address;   // не влезает в RAM вместе с RTOS
// TestTemperatureControl test_temperature_control;
// DateTimeTest date_time_test;
SetDateTime set_date_time;
// BatteryStation battery_station;

void setup() {
  // Вся инициализация приложения — в главном контексте до старта планировщика:
  // здесь delay() библиотек остаётся обычной паузой, ограничений RTOS нет.
  set_date_time.init();
  // battery_station.init();

  // Задача-планировщик с минимальным стеком; главный цикл ведёт idle-задача.
  xTaskCreate(schedulerTask, "sched", SCHEDULER_TASK_STACK_SIZE, nullptr, 1, nullptr);
  vTaskStartScheduler();

  // Сюда попадаем только при сбое запуска планировщика.
  for (;;) {
  }
}

// Работа главного цикла: вызывается из vApplicationIdleHook по флагу
// isPendingTask (раз в 500 мс). Собственный цикл Arduino не выполняется —
// планировщик не возвращает управление.
void loop() {
  // device_logic.loop();
  // test_ir_sensor.loop();
  // change_ir_sensor_address.loop();
  // test_temperature_control.loop();
  // date_time_test.loop();
  set_date_time.loop();
  // battery_station.loop();
}
