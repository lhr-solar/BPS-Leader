/** ===========================================================
Independent Watch Dog (IWDG) Driver
===============================================================
- The IWDG will trigger a reset sequence if it is not refreshed 
  within an expected time window. 
--------------------------------------------------------- */


#include "IWDG.h"
#include "stm32xx_hal.h"

/* IWDG struct */
IWDG_HandleTypeDef hiwdg = {0};

void IWDG_Init(GPIO_InitTypeDef gpio_config, void(*errorHandler)(void)) {
	// Check for previous reset
	if (IWDG_CheckIfReset() == 1) {
		errorHandler();
	}

	// Watchdog init
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALAR;
	hiwdg.Init.Reload = IWDG_COUNTDOWN;
	// hiwdg.Init.Window = 0x0FFF;				// Max window

	uint8_t init_status = HAL_IWDG_Init(&hiwdg);

	// GPIO init
	__HAL_RCC_GPIOA_CLK_ENABLE(); 
	HAL_GPIO_Init(GPIOA, &gpio_config);
	
	// Check IWDG init status	
	if (init_status!= HAL_OK) {
		errorHandler();
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


void IWDG_Error_Handler(void) {
	// __disable_irq(); 
	vTaskEndScheduler();

	GPIO_InitTypeDef led_config_f4 = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &led_config_f4);
	
	while (1) {
		/* If IWDG_Init fails, (for now) show blinky */
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(150);
	}
}