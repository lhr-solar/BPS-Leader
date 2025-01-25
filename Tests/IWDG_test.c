/* Watchdog Test
 - Attempts to pet watchdog within appropriate time interval
*/

#include "IWDG.h"
#include "stm32xx_hal.h"


void initLED_f4() {
    /* LED setup for stm32f4xx (LED is GPIOA, PIN_5) */
    GPIO_InitTypeDef led_config_f4 = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &led_config_f4);
}


int main() {
    HAL_Init();
    // initLED_f4();
    GPIO_InitTypeDef led_config_f4 = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };

    IWDG_Init(led_config_f4, IWDG_Error_Handler);
    // RESET CHECK
    if(IWDG_CheckIfReset() == 1) {
        while(1) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            HAL_Delay(100);
        }
    }

    // Set LED off to indicate we are in the init stage
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(500);
    
    while(1) {
        /* Refresh IWDG after a set timeout (system timeout) 
        *  - Toggle LED every refresh */
        HAL_Delay(8);
        IWDG_Refresh();
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); 
    }

    return 0;
}
