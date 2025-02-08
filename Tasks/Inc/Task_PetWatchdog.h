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
#include <event_groups.h>   /* Event group include */

#include "stm32xx_hal.h"
#include "IWDG.h"

// Task Parameters
#define TASK_PETWD_PRIORITY     tskIDLE_PRIORITY + 2
#define TASK_PETWD_STACK_SIZE   configMINIMAL_STACK_SIZE
extern StaticTask_t Task_PetWD_Buffer;
extern StackType_t Task_PetWD_Stack[configMINIMAL_STACK_SIZE];

// Event group
extern EventGroupHandle_t xEventGroupHandle;
extern StaticEventGroup_t xCreatedEventGroup;
extern EventBits_t uxBits;

// Dummy Tasks
#define DUM1_DONE   0x01
#define DUM2_DONE   0x02

/* TASK: Refreshes watchdog */
extern void Task_PetWatchdog(void *pvParameters);
extern uint8_t Flags;

#endif