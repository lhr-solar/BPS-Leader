#pragma once

#include "common.h"
#include <event_groups.h>

// Task configuration
#define TASK_INIT_PRIO                  tskIDLE_PRIORITY + 1
#define TASK_TEMPERATURE_MONITOR_PRIO   tskIDLE_PRIORITY + 3
#define TASK_VOLTAGE_MONITOR_PRIO       tskIDLE_PRIORITY + 4
#define TASK_AMPERES_MONITOR_PRIO       tskIDLE_PRIORITY + 5
#define TASK_PETWDOG_PRIO               tskIDLE_PRIORITY + 2
#define TASK_CAN_FORWARD_PRIO           tskIDLE_PRIORITY + 3

#define TEST_TASK_PRIORITY              tskIDLE_PRIORITY + 1 

// Task Stack Size 
#define TASK_INIT_STACK_SIZE                    configMINIMAL_STACK_SIZE
#define TASK_TEMPERATURE_MONITOR_STACK_SIZE     configMINIMAL_STACK_SIZE
#define TASK_VOLTAGE_MONITOR_STACK_SIZE         configMINIMAL_STACK_SIZE
#define TASK_AMPERES_MONITOR_STACK_SIZE         configMINIMAL_STACK_SIZE
#define TASK_PETWDOG_STACK_SIZE                 configMINIMAL_STACK_SIZE
#define PRECHARGE_TASK_STACK_SIZE               configMINIMAL_STACK_SIZE
#define FAULT_HANDLER_TASK_STACK_SIZE           configMINIMAL_STACK_SIZE
#define TASK_CAN_FORWARD_STACK_SIZE             configMINIMAL_STACK_SIZE

#define TEST_TASK_STACK_SIZE                    configMINIMAL_STACK_SIZE

// (exposed so that tests can init tasks)
// Task Stack Arrays 
extern StackType_t Task_Temperature_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
extern StackType_t Task_Voltage_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
extern StackType_t Task_Amperes_Stack_Array[ TASK_AMPERES_MONITOR_STACK_SIZE ];
extern StackType_t Task_Petwdog_Stack_Array[ TASK_PETWDOG_STACK_SIZE ];
extern StackType_t Precharge_Task_Stack[ PRECHARGE_TASK_STACK_SIZE ];
extern StackType_t FaultHandler_Task_Stack[ FAULT_HANDLER_TASK_STACK_SIZE ];
extern StackType_t Task_Can_Forward_Stack[ TASK_CAN_FORWARD_STACK_SIZE ];


// Task Buffers
extern StaticTask_t Task_Temperature_Buffer;
extern StaticTask_t Task_Voltage_Buffer;
extern StaticTask_t Task_Amperes_Buffer;
extern StaticTask_t Task_Petwdog_Buffer;
extern StaticTask_t Precharge_Task_Buffer;
extern StaticTask_t FaultHandler_Task_Buffer;
extern StaticTask_t Task_Can_Forward_Buffer;

// Task Delays
#define TEMP_MONITOR_TASK_DELAY_MS 10
#define VOLT_MONITOR_TASK_DELAY_MS 5
#define PRECHARGE_TASK_DELAY_MS 100

// Task Inits
void Task_Init();
void Task_Voltage_Monitor();
void Task_Temperature_Monitor();
void Task_PetWatchdog();
void Task_CanRxForward();

/* ---- Watchdog Event Group ---- */
void Init_WDogTask();
extern EventGroupHandle_t xWDogEventGroup_handle;
#define TEMP_MONITOR_DONE   (1 << 0)
#define VOLT_MONITOR_DONE   (1 << 1)
#define WINDOW_TIMER_DONE   (1 << 2)
#define ALL_TASKS_DONE (TEMP_MONITOR_DONE | VOLT_MONITOR_DONE | WINDOW_TIMER_DONE)

