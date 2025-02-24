#include "LEDs.h"

/**`
 * Inits the Heartbeat pin based on HEARTBEATPIN and HEARTBEATPORT
 * From PinConfig.h
*/
void Heartbeat_Init(){
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = HEARTBEATPIN
    };
    Heartbeat_Clock_Init();
    HAL_GPIO_Init(HEARTBEATPORT, &led_config);

}

/**
 * Inits the GPIO port clock based on HEARTBEATPORT
 * Can be called multiple times
 */
void Heartbeat_Clock_Init() {
    switch ((uint32_t)HEARTBEATPORT) {
        case (uint32_t)GPIOA:
            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;
        case (uint32_t)GPIOB:
            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;
        case (uint32_t)GPIOC:
            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;
        case (uint32_t)GPIOD:
            __HAL_RCC_GPIOD_CLK_ENABLE();
            break;
        case (uint32_t)GPIOE:
            __HAL_RCC_GPIOE_CLK_ENABLE();
            break;
    }
}


void Heartbeat_Toggle(){
    HAL_GPIO_TogglePin(HEARTBEATPORT, HEARTBEATPIN);
}


void Heartbeat_WritePin(uint8_t new_val){
    HAL_GPIO_WritePin(HEARTBEATPORT, HEARTBEATPIN, (new_val == GPIO_PIN_SET) ? GPIO_PIN_SET: GPIO_PIN_RESET);    
}