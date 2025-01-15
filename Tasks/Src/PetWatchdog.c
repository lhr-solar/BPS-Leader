// /* Watchdog Task
//  - Attempts to pet watchdog within appropriate time interval
// */

// #include "IWDG.h"
// #include "stm32xx_hal.h"
// #include "FreeRTOS.h"

// StaticTask_t task_buffer;
// StackType_t taskStack[configMINIMAL_STACK_SIZE];

// static void error_handler(void) {
//    while(1) {}
// }

// static void success_handler(void) {
//    GPIO_InitTypeDef led_init = {
//       .Mode = GPIO_MODE_OUTPUT_PP,
//       .Pull = GPIO_NOPULL,
//       .Pin = GPIO_PIN_5
//    };
    
//     __HAL_RCC_GPIOA_CLK_ENABLE();
//     HAL_GPIO_Init(GPIOA, &led_init);

//    while(1){
//       HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//       HAL_Delay(500);
//    }
// }

// static void Task_PetWatchdog(void *pvParameters) {
//     while(1) {
//         // if()
//         IWDG_Refresh();
//         HAL_Delay(8);
//     }
// }

// int main(void) {
//    if (HAL_Init() != HAL_OK) error_handler();

//    xTaskCreateStatic(
//                   task,
//                   "PetWatchdog",
//                   configMINIMAL_STACK_SIZE,
//                   NULL,
//                   tskIDLE_PRIORITY + 2,
//                   taskStack,
//                   &task_buffer);

//    vTaskStartScheduler();

//    error_handler();

//    return 0;
// }