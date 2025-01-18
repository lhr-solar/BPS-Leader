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

/* Blinks LED to signal we have faulted */
void error_handler(void) {
   // vTaskEndScheduler();
    while(1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(100);
    }
}