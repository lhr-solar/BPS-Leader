/* IWDG Test
 - Attempts to pet watchdog within appropriate time interval
*/

#include "IWDG.h"

int main() {
    HAL_Init();

    GPIO_InitTypeDef gpio_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &gpio_init);

    // Set LED off to indicate we are in the init stage
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(250);

    IWDG_Init();
    IWDG_Start(IWDG_Error_Handler);
    
    while(1) {
        /* Refresh IWDG after a set timeout: Toggle LED every refresh */
        HAL_Delay(15);
        IWDG_Refresh();
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); 
    }

    return 0;
}
