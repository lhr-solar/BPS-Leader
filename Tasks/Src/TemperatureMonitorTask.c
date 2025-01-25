#include "BPS_Tasks.h"
#include "stm32xx_hal.h"
#include "pinConfig.h"

void Task_Temperature_Monitor(){

        //Task deletes itself when all other tasks are Init'd
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = HEARTBEATPIN
    };
    
    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable clock for GPIOA
    HAL_GPIO_Init(GPIOA, &led_config); // initialize GPIOA with led_config

    while(1){
        HAL_GPIO_TogglePin(HEARTBEATPORT, HEARTBEATPIN);
        HAL_Delay(500);
    }
    
}