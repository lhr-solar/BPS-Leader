#include "common.h"
#include "StatusLEDs.h"
#include "Contactors.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "SHT45.h"
#include "EMC2305_Driver.h"
#include "DebugPrintf.h"
#include "BPS_Tasks.h"


int main(void) {
    HAL_Init();
    SystemClock_Config();

    Init_WDogTask();
    CAN_Init();
    LEDs_init();
    debugPrintf_init();
    

    xStateBits = xEventGroupCreateStatic(&xStateBits_buffer);
   
    xTaskCreateStatic(
            Task_Amperes_Monitor,                       /* The function that implements the task. */
            "Amperes Monitor Task",                     /* Text name for the task. */
            TASK_AMPERES_MONITOR_STACK_SIZE,            /* The size (in words) of the stack that should be created for the task. */
            (void*)NULL,                                /* Paramter passed into the task. */
            TASK_AMPERES_MONITOR_PRIO,                  /* Task Prioriy. */
            Task_Amperes_Stack_Array,                   /* Stack array. */
            &Task_Amperes_Buffer                        /* Buffer for static allocation. */
    );

        xTaskCreateStatic(
            Task_FaultHandler,                          /* The function that implements the task. */
            "Fault Handler Task",                       /* Text name for the task. */
            FAULT_HANDLER_TASK_STACK_SIZE,              /* The size (in words) of the stack that should be created for the task. */
            (void*)NULL,                                /* Paramter passed into the task. */
            TASK_FAULT_HANDLER_PRIO,                    /* Task Prioriy. */
            FaultHandler_Task_Stack,                    /* Stack array. */
            &FaultHandler_Task_Buffer                   /* Buffer for static allocation. */
    );

    
    // Start the scheduler
    vTaskStartScheduler();

    while(true){
        // Scheduler should've started by now
        // Code should never enter this point
    }

    return 0;
}

  