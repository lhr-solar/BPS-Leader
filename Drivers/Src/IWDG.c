/**
===========================================================
Independent Watch Dog (IWDG) Driver
===========================================================
- Description of watch dog coming soon



===========================================================
How to calculate COUNTDOWN value from desired refresh time
===========================================================
  - Countdown / timeout value is how long the IDWG will 
    count down before resetting the system. We think of this
    in terms of seconds / milliseconds, but the IWDG will 
    hold this value in terms of a tick countdown.


===========================================================
Formula [finding timeout (in s) from RL]
===========================================================
            t_IWDG = t_LSI * 4 * 2^PR * (RL + 1)
-----------------------------------------------------------
  Parameters:
  - t_IWDG  : IDWG timeout (in seconds)
  - t_LSI   : constant; 1/32,000 (represents 31.25 uS [micro second])
  - PR      : prescalar (PRESCALAR_4 = 0, PRESCALAR_8 = 1, etc)
  - RL      : reload time (in terms of IWDG ticks)


===========================================================
Revised formula [finding RL from timeout (in ms)]
===========================================================
      RL = [(t_IWDG * 32,000) / (4 * 2^PR * 1000)] - 1
-----------------------------------------------------------
  - RL      : countdown value to put for IWDG_COUNTDOWN
  - t_IWDG  : the countdown timeout you want in ms

*/


/* --------------------------------------------------------- */
#include "IWDG.h"
#include "stm32xx_hal.h"
// #include "stm32l4xx_hal.h"
// #include "stm32f4xx_hal.h"

/* Coundown value macro */
#define IWDG_COUNTDOWN 79   // 20ms

/* IWDG struct */
IWDG_HandleTypeDef hiwdg = {
  .Instance = IWDG
};


void IWDG_Init() {
    /* IDWG config */
    hiwdg.Init.Prescaler = IWDG_PRESCALER_8;
    hiwdg.Init.Reload = IWDG_COUNTDOWN;

    HAL_IWDG_Init(&hiwdg);

    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        Error_Handler();
    }
}


void IWDG_Refresh() {
  // __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
  // __HAL_IWDG_ENABLE_WRITE_ACCESS(&hiwdg);
  HAL_IWDG_Refresh(&hiwdg);
}


int IWDG_CheckIfReset() {
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
    __HAL_RCC_CLEAR_RESET_FLAGS();
    return 1;
  }
  return 0;
}


void Error_Handler(void) {
  __disable_irq(); 

  GPIO_InitTypeDef led_config = {
    .Mode = GPIO_MODE_OUTPUT_PP,
    .Pull = GPIO_NOPULL,
    .Pin = GPIO_PIN_3,
    .Speed = GPIO_SPEED_FREQ_LOW
  };
  __HAL_RCC_GPIOB_CLK_ENABLE(); 
  HAL_GPIO_Init(GPIOB, &led_config);

  while (1) {
    /* If IWDG_Init fails, turn off LED and (for now) halt program */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_Delay(500);
  }
}