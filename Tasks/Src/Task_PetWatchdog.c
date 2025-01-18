/* Pet Watchdog Task
 - Attempts to pet watchdog within appropriate time interval
*/

#include "BPS_Tasks.h"
#include "IWDG.h"

/*-----------------------------------------------------------*/

static void GPIO_Init() {
   /* LED: GPIO A, Pin 5*/
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };

   __HAL_RCC_GPIOA_CLK_ENABLE();
   HAL_GPIO_Init(GPIOA, &led_init);
}


// /* Blinks LED to signal we have faulted */
// static void error_handler(void) {
//    // vTaskEndScheduler();
//     while(1) {
//         HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//         HAL_Delay(100);
//     }
// }

/*-----------------------------------------------------------*/

/* TASK: Refreshes watchdog */
void Task_PetWatchdog() {
    GPIO_Init();
    
    // Don't fault on first run through
    static int i = 0;
    if(i > 0 && IWDG_CheckIfReset() == 1) {
        error_handler();
    }

    // Set LED off to indicate we are in the init stage
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(500);
    
    IWDG_Init();
    i++;

    // refresh within time limit for 50 cycles, then force watchdog to trip
    while(1) {
        // static int i = 1;
        IWDG_Refresh();
        // i > 50 ? HAL_Delay(20) : HAL_Delay(SYS_REFRESH_MS);
        HAL_Delay(SYS_REFRESH_MS);

        // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        // i++;
    }
}