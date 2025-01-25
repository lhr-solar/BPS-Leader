#include "LEDs.h"

void Heartbeat_Init(){
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = HEARTBEATPIN
    };
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(HEARTBEATPORT, &led_config);
    // TODO: init GPIO clocks
}

// TODO: burn this shit to the ground
void Heartbeat_Clock_Init(){
    if(HEARTBEATPORT == GPIOA){
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    if(HEARTBEATPORT == GPIOB){
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    if(HEARTBEATPORT == GPIOC){
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    if(HEARTBEATPORT == GPIOD){
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
}

void Heartbeat_Toggle(){
    HAL_GPIO_TogglePin(HEARTBEATPORT, HEARTBEATPIN);
}

void Heartbeat_WritePin(){

}