/* FreeRTOS includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "task.h" 
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "stm32xx_hal.h"
#include "BPS_Tasks.h"
#include "LEDs.h"

StaticTask_t pxTaskBufferRx;

StackType_t xStack[ configMINIMAL_STACK_SIZE ];

int main() {
    Heartbeat_Init();
    xTaskCreateStatic( /* The function that implements the task. */
                    Task_Init,
                    /* Text name for the task, just to help debugging. */
                    "Init Task",
                    configMINIMAL_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
                    (void*)NULL, /* Paramter passed into the task */
                    TASK_INIT_PRIO, /* Task Prioriy */
                    xStack, /* Stack array */
                    &pxTaskBufferRx  /* Buffer for static allocation */
   );

    Heartbeat_Toggle();
    
    // Start the scheduler
    vTaskStartScheduler();

    while(1){
        // Scheduler should've started by now
        // Code should never enter this point
    }
    
    return 0;
}
