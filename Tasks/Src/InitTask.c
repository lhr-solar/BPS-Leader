#include "Tasks.h"

void Task_Init(){

    xTaskCreateStatic(
      Task_Init,                        /* Function that implements the task. */
      "Task_Voltage_Monitor",           /* Text name for the task. */
      TASK_VOLTAGE_MONITOR_STACK_SIZE,  /* Stack size in words. */
      NULL,                             /* Parameter passed into the task. */
      TASK_VOLTAGE_MONITOR_PRIO,        /* Task Priority. */
      NULL,                             /* Task Handle. */
      NULL );

    xTaskCreateStatic(
      Task_Init,                            /* Function that implements the task. */
      "Task_Temperature_Monitor",           /* Text name for the task. */
      TASK_TEMPERATURE_MONITOR_STACK_SIZE,  /* Stack size in words. */
      NULL,                                 /* Parameter passed into the task. */
      TASK_TEMPERATURE_MONITOR_PRIO,        /* Task Priority. */
      NULL,                                 /* Task Handle. */
      NULL );


    // Task deletes itself when all other tasks are Init'd
    vTaskDelete(NULL);

    
}