/** ===========================================================
  Watchdog Driver
===============================================================
- The Watchdog will trigger a reset sequence if it is not refreshed 
  within an expected time window. 
--------------------------------------------------------- */

#include "WDog.h"

/* Watchdog struct */
IWDG_HandleTypeDef wdog = {0};	// Independent Watchdog


void WDog_Init(void(*errorHandler)(void)) {
	if (WDog_CheckIfReset() == 1) {
		errorHandler();
	}
	
	// Set IWDG values
	wdog.Instance = IWDG;
	wdog.Init.Prescaler = WDOG_PRESCALAR;
	wdog.Init.Reload = WDOG_COUNTDOWN;

	// Initialize IWDG and check init status
	if (HAL_IWDG_Init(&wdog) != HAL_OK) {
		errorHandler();
	}
}


HAL_StatusTypeDef WDog_Refresh() {
	return HAL_IWDG_Refresh(&wdog);
}


uint8_t WDog_CheckIfReset() {
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) == SET) {
		__HAL_RCC_CLEAR_RESET_FLAGS();
		return 1;
	}
	return 0;
}


void WDog_Error_Handler(void) {
	if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
		vTaskEndScheduler();	// jumps back to when vTaskStartScheduler() was called, so nothing past this actually runs :(
		// vTaskSuspendAll();	// just suspends, can still run stuff after this?
	}

	GPIO_InitTypeDef led_config_f4 = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &led_config_f4);
	
	while (1) {
		/* If Watchdog init fails, (for now) show blinky */
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(150);
	}
}
