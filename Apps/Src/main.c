#include "FreeRTOS.h" /* Must come first. */
#include "task.h" 
#include "BPS_Tasks.h"

// Task Stack Arrays
StackType_t Task_Init_Stack_Array[ configMINIMAL_STACK_SIZE ];
// StackType_t Task_Temperature_Stack_Array[ configMINIMAL_STACK_SIZE ];
// StackType_t Task_Voltage_Stack_Array[ configMINIMAL_STACK_SIZE ];
// StackType_t Task_Amperes_Stack_Array[ configMINIMAL_STACK_SIZE ];
// StackType_t Task_Petwdog_Stack_Array[ configMINIMAL_STACK_SIZE ];

// // Task Buffers
StaticTask_t Task_Init_Buffer;
// StaticTask_t Task_Temperature_Buffer;
// StaticTask_t Task_Voltage_Buffer;
// StaticTask_t Task_Amperes_Buffer;
// StaticTask_t Task_Petwdog_Buffer;



int main() {
    xTaskCreateStatic(
                    Task_Init, /* The function that implements the task. */
                    "Init Task", /* Text name for the task. */
                    configMINIMAL_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
                    (void*)NULL, /* Paramter passed into the task. */
                    TASK_INIT_PRIO, /* Task Prioriy. */
                    Task_Init_Stack_Array, /* Stack array. */
                    &Task_Init_Buffer  /* Buffer for static allocation. */
   );

//     // Start the scheduler
    vTaskStartScheduler();

    while(1){
        // Scheduler should've started by now
        // Code should never enter this point
    }
    
    return 0;
}
