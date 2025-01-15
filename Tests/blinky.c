#include "stm32xx_hal.h"

/* COPIED FROM EMBEDDED SHAREPOINT TEST */

int main(){
    HAL_Init();

    /* LED setup: GPIOA, PIN_5 */
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           // enable clock for GPIOA
    HAL_GPIO_Init(GPIOA, &led_config); // initialize GPIOA with led_config

    while(1){
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(500);
    }

    return 0;
}