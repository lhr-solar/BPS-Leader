#include "common.h"
#include "StatusLEDs.h"
#include "Contactors.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "SHT45.h"
#include "EMC2305_Driver.h"

StaticTask_t Task_Init_Buffer;
StackType_t Task_Init_Stack_Array[ TASK_INIT_STACK_SIZE ];

int main(void) {

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

    while(true){
        // Scheduler should've started by now
        // Code should never enter this point
    }

    return 0;
}

  