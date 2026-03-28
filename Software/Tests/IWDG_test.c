/* IWDG Test
 - Attempts to pet watchdog within appropriate time interval
*/

#include "IWDG.h"

int main() {
    HAL_Init();

    SystemClock_Config();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


    // Set LED off to indicate we are in the init stage
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_Delay(250);

    IWDG_Init();
    IWDG_Start(IWDG_Error_Handler);
    
    while(1) {
        /* Refresh IWDG after a set timeout: Toggle LED every refresh */
        HAL_Delay(15);
        IWDG_Refresh();
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3); 
    }

    return 0;
}