/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "task.h" /* RTOS task related API prototypes. */
#include "queue.h" /* RTOS queue related API prototypes. */
#include "timers.h" /* Software timer related API prototypes. */
#include "semphr.h" /* Semaphore related API prototypes. */

#include "stm32xx_hal.h"
#include "TaskInit.h"

int main() {

        xTaskCreateStatic( /* The function that implements the task. */
                    Task_Init,
                    /* Text name for the task, just to help debugging. */
                    "Task_Init",
                    /* The size (in words) of the stack that should be created
                    for the task. */
                    configMINIMAL_STACK_SIZE,
                 /* A parameter that can be passed into the task. Not used
                    in this simple demo. */
                    NULL,
                 /* The priority to assign to the task. tskIDLE_PRIORITY
                    (which is 0) is the lowest priority. configMAX_PRIORITIES - 1
                    is the highest priority. */
                    tskIDLE_PRIORITY,
                 /* Used to obtain a handle to the created task. Not used in
                    this simple demo, so set to NULL. */
                    NULL,
                    /* Buffer for static allocation */
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
