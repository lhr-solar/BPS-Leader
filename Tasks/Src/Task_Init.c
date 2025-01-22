/*-----------------------------------------------------------
 * INIT TASK
 - Initializes all tasks
/*-----------------------------------------------------------*/
#include "BPS_Tasks.h"

void Task_Init() {
    if (HAL_Init() != HAL_OK) {
        error_handler();
    }
    
    xTaskCreateStatic(  Task_PetWatchdog,
                        "PetWatchdog",
                        TASK_PETWD_STACK_SIZE,
                        NULL,
                        TASK_PETWD_PRIORITY,
                        Task_PetWD_Stack,
                        &Task_PetWD_Buffer);

    xTaskCreateStatic(  Task_DummyTask,
                        "DummyTask1",
                        TASK_DUMMY_STACK_SIZE,
                        NULL,
                        TASK_DUMMY_PRIORITY,
                        Task_Dummy_Stack,
                        &Task_Dummy_Buffer);
    
    // delete this task once all other tasks are initialized
    vTaskDelete(NULL);
}