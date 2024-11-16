#include "IWDG.h"
#include "stm32xx_hal.h"
// #include <stm32l4xx_hal_gpio.h>
// #include "stm32l4xx_hal_rcc.h"

int main() {
    HAL_Init();

    /* LED config for stm32l432kcu (GPIOB, PIN_3)*/
    // GPIO_InitTypeDef led_config_l432 = {
    //     .Mode = GPIO_MODE_OUTPUT_PP,
    //     .Pull = GPIO_NOPULL,
    //     .Pin = GPIO_PIN_3
    //     // , .Speed = GPIO_SPEED_FREQ_LOW
    // };
    // __HAL_RCC_GPIOB_CLK_ENABLE();           // enable clock for GPIOB
    // HAL_GPIO_Init(GPIOB, &led_config_l432); // initialize GPIOA with led_config
        
    /* LED setup for stm32f4xx (LED is GPIOA, PIN_5) */
    GPIO_InitTypeDef led_config_f4 = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    __HAL_RCC_GPIOA_CLK_ENABLE();       // enable clock for GPIOB
    HAL_GPIO_Init(GPIOA, &led_config_f4);  // initialize GPIOA with led_config

    if(IWDG_CheckIfReset() == 1) {
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET); // l432kcu
        // while(1) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // f4

        HAL_Delay(1000);
        // }
    }
    
    IWDG_Init();
    
    while(1) {
        /* LED feedback (constantly on if IWDG is refreshed in time) */
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); // l432kcu
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // f4


        /* IWDG test */
        HAL_Delay(258);   // must refresh faster than IWDG timeout 
        /**
         *  Weird issues :(
         * - supposedly 5 ms timeout:   IWDG resets with >= 2 ms delay
         * - supposedly 50 ms timeout:  IWDG resets with >= 25 ms delay
         * - supposedly 500 ms timeout: IWDG resets with >= 258 ms delay
         */ 
        
        IWDG_Refresh();
    }

    return 0;
}