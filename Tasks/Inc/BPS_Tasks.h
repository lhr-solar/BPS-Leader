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
#define TASK_INIT_PRIO                      tskIDLE_PRIORITY + 2
#define TASK_TEMPERATURE_MONITOR_PRIO       tskIDLE_PRIORITY + 3
#define TASK_VOLTAGE_MONITOR_PRIO           tskIDLE_PRIORITY + 4
#define TASK_AMPERES_MONITOR_PRIO           tskIDLE_PRIORITY + 5
#define TASK_PETWDOG_PRIO                   tskIDLE_PRIORITY + 6

// Task Stack Size 
#define TASK_INIT_STACK_SIZE                       configMINIMAL_STACK_SIZE
#define TASK_TEMPERATURE_MONITOR_STACK_SIZE        configMINIMAL_STACK_SIZE
#define TASK_VOLTAGE_MONITOR_STACK_SIZE            configMINIMAL_STACK_SIZE
#define TASK_AMPERES_MONITOR_STACK_SIZE            configMINIMAL_STACK_SIZE
#define TASK_PETWDOG_STACK_SIZE                    configMINIMAL_STACK_SIZE


// Task Stack Arrays
StackType_t Task_Init_Stack_Array[ configMINIMAL_STACK_SIZE ];
StackType_t Task_Temperature_Stack_Array[ configMINIMAL_STACK_SIZE ];
StackType_t Task_Voltage_Stack_Array[ configMINIMAL_STACK_SIZE ];
StackType_t Task_Amperes_Stack_Array[ configMINIMAL_STACK_SIZE ];
StackType_t Task_Petwdog_Stack_Array[ configMINIMAL_STACK_SIZE ];

// Task Buffers
StaticTask_t Task_Init_Buffer;
StaticTask_t Task_Temperature_Buffer;
StaticTask_t Task_Voltage_Buffer;
StaticTask_t Task_Amperes_Buffer;
StaticTask_t Task_Petwdog_Buffer;

// Task Inits
void Task_Init();
void Task_Voltage_Monitor();
void Task_Temperature_Monitor();
void Task_PETWDOG();

#endif