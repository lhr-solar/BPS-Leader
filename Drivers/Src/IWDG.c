#include "IWDG.h"
// #include "stm32xx_hal.h"
// #include "stm32f4xx_hal.h"

/* Coundown value macro */
#define IWDG_COUNTDOWN 39     // 5ms
#define IWDG_REFRESH_TIME 4   // 4ms

IWDG_HandleTypeDef hiwdg;

void IWDG_Init() {
    // HAL_Init();

    // LED
    // GPIO_InitTypeDef led_config = {
    //     .Mode = GPIO_MODE_OUTPUT_PP,
    //     .Pull = GPIO_NOPULL,
    //     .Pin = GPIO_PIN_5
    // };
    // __HAL_RCC_GPIOA_CLK_ENABLE();       // enable clock for GPIOA
    // HAL_GPIO_Init(GPIOA, &led_config);  // initialize GPIOA with led_config

    // IWDG parameters
    // IWDG_HandleTypeDef hiwdg;
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
    hiwdg.Init.Reload = IWDG_COUNTDOWN;

    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        Error_Handler();
    }

    HAL_IWDG_Init(&hiwdg);
}

void IWDG_Refresh() {
  __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
  __HAL_IWDG_ENABLE_WRITE_ACCESS(&hiwdg);
}


void IWDG_Reset() {
  __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
}

void IWDG_CheckStatus() {

}

void Error_Handler(void) {
  /* Users can add their own implementation to report the HAL error return state */
  __disable_irq();

  while (1) {
    // printf("iwdg initialization failed");
  }
}