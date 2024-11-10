#include "IWDG.h"
#include "stm32xx_hal.h"
#include <stm32l4xx_hal_gpio.h>
#include "stm32l4xx_hal_rcc.h"

// #include "stm32l4xx_hal.h"

int main() {
    HAL_Init();

    // init led 
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_3,
        .Speed = GPIO_SPEED_FREQ_LOW
    };
    
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable clock for GPIOA

    HAL_GPIO_Init(GPIOB, &led_config); // initialize GPIOA with led_config
    
    // add a better way to detect if watchdog has tripped
        // turn off LED if watchdog has tripped
        // if (_HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) == RESET) {
            // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        // }
        
        // HAL_Delay(200);
    
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);

    // IWDG_Init();

    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);

    while(1){
        /* Blinky test (works) */
        // HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
        // HAL_Delay(200);   
        
        /* IWDG test */
        // HAL_Delay(5);   // must refresh faster than 5 ms
        // IWDG_Refresh();
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    }

    return 0;
}