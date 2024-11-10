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


// Task Inits
void Task_Init();

#endif