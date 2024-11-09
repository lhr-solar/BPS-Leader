#include "IWDG.h"
#include "stm32xx_hal.h"
#include "stm32f4xx_hal.h"

/* Coundown value macro */
#define IWDG_COUNTDOWN

void IWDG_Init() {
    HAL_Init();

    /* Initialize LED */
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    __HAL_RCC_GPIOA_CLK_ENABLE();       // enable clock for GPIOA
    HAL_GPIO_Init(GPIOA, &led_config);  // initialize GPIOA with led_config

    /* IWDG Parameters */
    IWDG_HandleTypeDef hiwdg;
    // hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
    hiwdg.Init.Reload = 39;

    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        Error_Handler();
    }

    IWDG_REFRESH_TIME = 4; // ms
    
    // System refresh ???
    SysTick_Config(5);
    // SystemClock_Config();

    HAL_IWDG_Init(&hiwdg);
}


    // while(1) {
    //     HAL_Delay(IDWG_KICK_TIME);
    //     HAL_IWDG_Refresh(&hiwdg);
    // }

    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    // HAL_Delay(500);


void Error_Handler(void) {
  /* Users can add their own implementation to report the HAL error return state */
  __disable_irq();

  while (1) {
    // printf("iwdg initialization failed");
  }

}