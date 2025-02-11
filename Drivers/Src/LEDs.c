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
    GPIO_CLK_ENABLE((uint32_t)HEARTBEATPORT);
}


void Heartbeat_Toggle(){
    HAL_GPIO_TogglePin(HEARTBEATPORT, HEARTBEATPIN);
}


void Heartbeat_WritePin(uint8_t new_val){
    HAL_GPIO_WritePin(HEARTBEATPORT, HEARTBEATPIN, (new_val == GPIO_PIN_SET) ? GPIO_PIN_SET: GPIO_PIN_RESET);    
}