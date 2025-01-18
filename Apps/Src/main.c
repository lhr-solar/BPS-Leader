#include "BPS_Tasks.h"

int main() {
    // if (HAL_Init() != HAL_OK) {
    //     error_handler();
    // }

    xTaskCreateStatic(  Task_Init,   
                        "Init",
                        TASK_INIT_STACK_SIZE,
                        NULL,
                        TASK_INIT_PRIORITY,
                        Task_Init_Stack,
                        &Task_Init_Buffer);

    vTaskStartScheduler();
    // scheduler started, code should not reach here
    error_handler(); // replace with something more useful to this situation

    return 0;
}