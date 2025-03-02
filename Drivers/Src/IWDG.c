/** ===========================================================
 Independent Watchdog (IWDG)
===============================================================
The Watchdog will trigger a reset sequence if it is not refreshed 
within an expected time window. 
--------------------------------------------------------- */

#include "IWDG.h"

/* IWDG struct */
IWDG_HandleTypeDef iwdg_h = {0};


void IWDG_Init() {
    // Set IWDG values
    iwdg_h.Instance = IWDG;
    iwdg_h.Init.Prescaler = IWDG_PRESCALAR;
    iwdg_h.Init.Reload = IWDG_COUNTDOWN;
}

void IWDG_Start(void(*errorHandler)(void)) {
    // Check for previous reset
    if (IWDG_CheckIfReset() == 1) {
        errorHandler();
    }

    // Initialize / start IWDG and check init status
    if (HAL_IWDG_Init(&iwdg_h) != HAL_OK) {
        errorHandler();
    }
}

HAL_StatusTypeDef IWDG_Refresh() {
    return HAL_IWDG_Refresh(&iwdg_h);
}


uint8_t IWDG_CheckIfReset() {
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) == SET) {
        __HAL_RCC_CLEAR_RESET_FLAGS();
        return 1;
    }
    return 0;
}


void IWDG_Error_Handler(void) {
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskEndScheduler();	// jumps back to when vTaskStartScheduler() was called, so nothing past this actually runs :(
        // vTaskSuspendAll();	// just suspends, can still run stuff after this?
    }
    while (1) {}
}
