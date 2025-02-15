#ifndef TASK_H__
#define TASK_H__
/* Kernel includes. */
#include "FreeRTOS.h"       /* Must come first. */
#include "task.h"           /* RTOS task related API prototypes. */
#include "queue.h"          /* RTOS queue related API prototypes. */
#include "timers.h"         /* Software timer related API prototypes. */
#include "semphr.h"         /* Semaphore related API prototypes. */
#include <event_groups.h>   /* Event groups */

#include "stm32xx_hal.h"

// Task Priority 
#define TASK_INIT_PRIO                  tskIDLE_PRIORITY + 1
#define TASK_TEMPERATURE_MONITOR_PRIO   tskIDLE_PRIORITY + 3
#define TASK_VOLTAGE_MONITOR_PRIO       tskIDLE_PRIORITY + 4
#define TASK_AMPERES_MONITOR_PRIO       tskIDLE_PRIORITY + 5
#define TASK_PETWDOG_PRIO               tskIDLE_PRIORITY + 2

// Task Stack Size 
#define TASK_INIT_STACK_SIZE                    configMINIMAL_STACK_SIZE
#define TASK_TEMPERATURE_MONITOR_STACK_SIZE     configMINIMAL_STACK_SIZE
#define TASK_VOLTAGE_MONITOR_STACK_SIZE         configMINIMAL_STACK_SIZE
#define TASK_AMPERES_MONITOR_STACK_SIZE         configMINIMAL_STACK_SIZE
#define TASK_PETWDOG_STACK_SIZE                 configMINIMAL_STACK_SIZE

// (exposed so that tests can init tasks)
// Task Stack Arrays 
extern StackType_t Task_Temperature_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
extern StackType_t Task_Voltage_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
extern StackType_t Task_Amperes_Stack_Array[ TASK_AMPERES_MONITOR_STACK_SIZE ];
extern StackType_t Task_Petwdog_Stack_Array[ TASK_PETWDOG_STACK_SIZE ];

// Task Buffers
extern StaticTask_t Task_Temperature_Buffer;
extern StaticTask_t Task_Voltage_Buffer;
extern StaticTask_t Task_Amperes_Buffer;
extern StaticTask_t Task_Petwdog_Buffer;

// Event group
extern EventGroupHandle_t xEventGroupHandle;
extern StaticEventGroup_t xCreatedEventGroup;
extern EventBits_t uxBits;

// // Watchdog window timer
// extern TimerHandle_t xWindowTimer;
// extern StaticTimer_t xTimerBuffer;

// Dummy Tasks
#define TASK1_BIT   0x01
#define TASK2_BIT   0x02
#define ALL_TASKS_BITS (TASK1_BIT | TASK2_BIT)

// Task Inits
void Task_Init();
void Task_Voltage_Monitor();
void Task_Temperature_Monitor();
void Task_PETWDOG();

// Watchdog
void Init_WDogEventGroup();

#endif