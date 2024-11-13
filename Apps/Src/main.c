/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "task.h" /* RTOS task related API prototypes. */
#include "queue.h" /* RTOS queue related API prototypes. */
#include "timers.h" /* Software timer related API prototypes. */
#include "semphr.h" /* Semaphore related API prototypes. */

#include "stm32xx_hal.h"
#include "Tasks.h"
#include "LEDs.h"

int main() {
    //Heartbeat_Init();
    //Heartbeat_Toggle();

   xTaskCreateStatic(
      Task_Init,            /* Function that implements the task. */
      "Task_Init",          /* Text name for the task. */
      TASK_INIT_STACK_SIZE, /* Stack size in words. */
      NULL,                 /* Parameter passed into the task. */
      TASK_INIT_PRIO,       /* Task Priority. */
      NULL,                  /* Task Handle. */
      NULL );

    // Start the scheduler
    vTaskStartScheduler();
    
    
    // start the scheduler
    while(1){
        // Scheduler should've started by now
        // Code should never enter this point
    }
    
    return 0;
}
