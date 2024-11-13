/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "task.h" /* RTOS task related API prototypes. */
#include "queue.h" /* RTOS queue related API prototypes. */
#include "timers.h" /* Software timer related API prototypes. */
#include "semphr.h" /* Semaphore related API prototypes. */

#include "stm32xx_hal.h"
#include "Tasks.h"
#include "LEDs.h"

StaticTask_t pxTaskBufferRx;

int main() {
    Heartbeat_Init();
    Heartbeat_Toggle();


    /* Create the queue receive task as described in the comments at the top
    of this file. */
    xTaskCreateStatic( /* The function that implements the task. */
                    Task_Init,
                    /* Text name for the task, just to help debugging. */
                    "Init Task",
                    /* The size (in words) of the stack that should be created
                    for the task. */
                    configMINIMAL_STACK_SIZE,
                 /* A parameter that can be passed into the task. Not used
                    in this simple demo. */
                    NULL,
                 /* The priority to assign to the task. tskIDLE_PRIORITY
                    (which is 0) is the lowest priority. configMAX_PRIORITIES - 1
                    is the highest priority. */
                    TASK_INIT_PRIO,
                 /* Used to obtain a handle to the created task. Not used in
                    this simple demo, so set to NULL. */
                    NULL,
                    /* Buffer for static allocation */
                    &pxTaskBufferRx );

    //Heartbeat_Toggle();
    //Heartbeat_Toggle();
    // Start the scheduler
    vTaskStartScheduler();

    // start the scheduler
    while(1){
        //Heartbeat_Toggle();
        // Scheduler should've started by now
        // Code should never enter this point
        HAL_Delay(100);
    }
    
    return 0;
}
