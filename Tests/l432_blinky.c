#include "stm32xx_hal.h"

/* COPIED FROM EMBEDDED SHAREPOINT TEST */

int main(){
    HAL_Init();

    /* LED setup for stm32f4xx (LED is GPIOA, PIN_5) */
    // GPIO_InitTypeDef led_config_f4 = {
    //     .Mode = GPIO_MODE_OUTPUT_PP,
    //     .Pull = GPIO_NOPULL,
    //     .Pin = GPIO_PIN_5
    // };
    // __HAL_RCC_GPIOA_CLK_ENABLE();       // enable clock for GPIOB
    // HAL_GPIO_Init(GPIOA, &led_config_f4);  // initialize GPIOA with led_config

    /* LED setup for stm32l432kcu (LED is GPIOB, PIN_3)*/
    GPIO_InitTypeDef led_config_l432 = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_3
    };
    __HAL_RCC_GPIOB_CLK_ENABLE();           // enable clock for GPIOB
    HAL_GPIO_Init(GPIOB, &led_config_l432); // initialize GPIOA with led_config

    while(1){
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
        HAL_Delay(500);
    }

    return 0;
}