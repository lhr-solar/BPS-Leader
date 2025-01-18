#ifndef BPS_TASKS_H
#define BPS_TASKS_H

#include "FreeRTOS.h"   /* Must come first. */
#include "task.h"       /* RTOS task related API prototypes. */
#include "queue.h"      /* RTOS queue related API prototypes. */
#include "timers.h"     /* Software timer related API prototypes. */
#include "semphr.h"     /* Semaphore related API prototypes. */
#include "stm32xx_hal.h"

/*-----------------------------------------------------------*/

// Task Priorities
#define TASK_INIT_PRIORITY      tskIDLE_PRIORITY + 2
#define TASK_PETWD_PRIORITY     tskIDLE_PRIORITY + 5
#define TASK_DUMMY_PRIORITY     tskIDLE_PRIORITY + 5
#define TASK_DUMMY2_PRIORITY    tskIDLE_PRIORITY + 5

// Task Stack Sizes
#define TASK_INIT_STACK_SIZE    configMINIMAL_STACK_SIZE
#define TASK_PETWD_STACK_SIZE   configMINIMAL_STACK_SIZE
#define TASK_DUMMY_STACK_SIZE   configMINIMAL_STACK_SIZE
#define TASK_DUMMY2_STACK_SIZE  configMINIMAL_STACK_SIZE

// Task Buffers
extern StaticTask_t Task_Init_Buffer;
extern StaticTask_t Task_PetWD_Buffer;
extern StaticTask_t Task_Dummy_Buffer;
extern StaticTask_t Task_Dummy2_Buffer;

// Task Stack Arrays
extern StackType_t Task_Init_Stack[configMINIMAL_STACK_SIZE];
extern StackType_t Task_PetWD_Stack[configMINIMAL_STACK_SIZE];
extern StackType_t Task_Dummy_Stack[configMINIMAL_STACK_SIZE];
extern StackType_t Task_Dummy2_Stack[configMINIMAL_STACK_SIZE];

// Tasks
void Task_Init();
void Task_PetWatchdog();
void Task_DummyTask();


#endif