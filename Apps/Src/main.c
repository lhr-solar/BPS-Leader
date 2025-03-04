#include "FreeRTOS.h" /* Must come first. */
#include "task.h" 
#include "BPS_Tasks.h"
#include "pinConfig.h"
#include "stm32xx_hal.h"

StaticTask_t Task_Init_Buffer;
StackType_t Task_Init_Stack_Array[ TASK_INIT_STACK_SIZE ];

int main() {
    HAL_Init();
    SystemClock_Config();
    
    xTaskCreateStatic(
                    Task_Init, /* The function that implements the task. */
                    "Init Task", /* Text name for the task. */
                    configMINIMAL_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
                    (void*)NULL, /* Paramter passed into the task. */
                    TASK_INIT_PRIO, /* Task Prioriy. */
                    Task_Init_Stack_Array, /* Stack array. */
                    &Task_Init_Buffer  /* Buffer for static allocation. */
   );
    // Start the scheduler
    vTaskStartScheduler();

    while(1){
        // Scheduler should've started by now
        // Code should never enter this point
    }
    
    return 0;
}
