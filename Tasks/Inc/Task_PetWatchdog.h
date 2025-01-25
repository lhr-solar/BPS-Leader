/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/

#ifndef TASK_PETWATCHDOG_H
#define TASK_PETWATCHDOG_H

// Kernel Includes
#include "FreeRTOS.h"   /* Must come first. */
#include "task.h"       /* RTOS task related API prototypes. */
#include "queue.h"      /* RTOS queue related API prototypes. */
#include "timers.h"     /* Software timer related API prototypes. */
#include "semphr.h"     /* Semaphore related API prototypes. */
#include "stm32xx_hal.h"

#include "IWDG.h"

// Task Parameters
#define TASK_PETWD_PRIORITY     tskIDLE_PRIORITY + 1
#define TASK_PETWD_STACK_SIZE   configMINIMAL_STACK_SIZE
StaticTask_t Task_PetWD_Buffer;
StackType_t Task_PetWD_Stack[configMINIMAL_STACK_SIZE];

// semaphore
extern SemaphoreHandle_t xIWDG_Semaphore;
extern StaticSemaphore_t xIWDG_SemaphoreBuffer;

/* TASK: Refreshes watchdog */
void Task_PetWatchdog();

#endif