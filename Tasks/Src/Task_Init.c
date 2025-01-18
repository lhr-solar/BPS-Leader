/* Initialization Task
 - Inits all tasks
*/

#include "BPS_Tasks.h"

// Task Buffers
StaticTask_t Task_Init_Buffer;
StaticTask_t Task_PetWD_Buffer;
StaticTask_t Task_Dummy_Buffer;
StaticTask_t Task_Dummy2_Buffer;

// Task Stack Arrays
StackType_t Task_Init_Stack[configMINIMAL_STACK_SIZE];
StackType_t Task_PetWD_Stack[configMINIMAL_STACK_SIZE];
StackType_t Task_Dummy_Stack[configMINIMAL_STACK_SIZE];
StackType_t Task_Dummy2_Stack[configMINIMAL_STACK_SIZE];

void Task_Init() {
    // /* BLINKY TEST */
    // HAL_Init();
    // GPIO_InitTypeDef led_config = {
    //     .Mode = GPIO_MODE_OUTPUT_PP,
    //     .Pull = GPIO_NOPULL,
    //     .Pin = GPIO_PIN_5
    // };
    // __HAL_RCC_GPIOA_CLK_ENABLE();           // enable clock for GPIOA
    // HAL_GPIO_Init(GPIOA, &led_config); // initialize GPIOA with led_config
    // while(1){
    //     HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    //     HAL_Delay(50);
    // }

   xTaskCreateStatic(   Task_PetWatchdog,
                        "PetWatchdog",
                        TASK_PETWD_STACK_SIZE,
                        NULL,
                        TASK_PETWD_PRIORITY,
                        Task_PetWD_Stack,
                        &Task_PetWD_Buffer);

//    xTaskCreateStatic(
//                     Task_DummyTask,
//                     "DummyTask1",
//                     TASK_DUMMY_STACK_SIZE,
//                     NULL,
//                     TASK_DUMMY_PRIORITY,
//                     Task_Dummy_Stack,
//                     &Task_Dummy_Buffer);

    
//     // delete this task once all other tasks are initialized
    vTaskDelete(NULL);
}