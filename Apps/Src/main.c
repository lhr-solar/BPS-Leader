#include "BPS_Tasks.h"

int main() {
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

  xTaskCreateStatic(
                Task_Init,   
                "Init",
                TASK_INIT_STACK_SIZE,
                NULL,
                TASK_INIT_PRIORITY,
                Task_Init_Stack,
                &Task_Init_Buffer);

  vTaskStartScheduler();

  return 0;
}