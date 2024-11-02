#include "main.h"
#include "stm32xx_hal.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

int main(void) {
    HAL_Init();

    int counter = 0;

    // IWDG (Independent WatchDog) parameters
    IWDG_HandleTypeDef hiwdg;
    // hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
    hiwdg.Init.Reload = 39;

    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        Error_Handler();
    }

    IDWG_KICK_TIME = 4; // ms
    
    // System refresh ???
    SysTick_Config(5);
    // SystemClock_Config();

    HAL_IWDG_Init(&hiwdg);

    while(1) {
        HAL_Delay(IDWG_KICK_TIME);
        HAL_IWDG_Refresh(&hiwdg);
        counter++;
        printf("%d", counter);
    }

    return 0;
}


void Error_Handler(void) {
  /* Users can add their own implementation to report the HAL error return state */
  __disable_irq();

  while (1) {
    printf("iwdg initialization failed");
  }

}