#ifndef TASK_H__
#define TASK_H__

/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "task.h" /* RTOS task related API prototypes. */
#include "queue.h" /* RTOS queue related API prototypes. */
#include "timers.h" /* Software timer related API prototypes. */
#include "semphr.h" /* Semaphore related API prototypes. */

#include "stm32xx_hal.h"

// Task Priority 
#define TASK_INIT_PRIO                      0
#define TASK_TEMPERATURE_MONITOR_PRIO       1
#define TASK_VOLTAGE_MONITOR_PRIO           2
#define TASK_AMPERES_MONITOR_PRIO           3
#define TASK_PETWDOG_PRIO                   4

// Task Stack Size 
#define TASK_INIT_STACK_SIZE                       configMINIMAL_STACK_SIZE
#define TASK_TEMPERATURE_MONITOR_STACK_SIZE        configMINIMAL_STACK_SIZE
#define TASK_VOLTAGE_MONITOR_STACK_SIZE            configMINIMAL_STACK_SIZE
#define TASK_AMPERES_MONITOR_STACK_SIZE            configMINIMAL_STACK_SIZE
#define TASK_PETWDOG_STACK_SIZE                    configMINIMAL_STACK_SIZE



// Task Inits
void Task_Init();
void Task_Voltage_Monitor();
void Task_Temperature_Monitor();
void Task_PETWDOG();

#endif