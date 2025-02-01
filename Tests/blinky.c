#include "stm32xx_hal.h"
#include "pinConfig.h"
#include "LEDs.h"

int main(){
    HAL_Init();
    Heartbeat_Init();

    while(1){
        Heartbeat_Toggle();
        HAL_Delay(100);
    }

    return 0;
}