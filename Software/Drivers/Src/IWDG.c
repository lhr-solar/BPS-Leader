/** ===========================================================
 Independent Watchdog (IWDG)
===============================================================
The Watchdog will trigger a reset sequence if it is not refreshed 
within an expected time window. 
--------------------------------------------------------- */
#include "common.h"
#include "IWDG.h"
#include "faultHandler.h"

/* IWDG struct */
IWDG_HandleTypeDef iwdg_h = {0};


void IWDG_Init() {
    // Set IWDG values. Hardware windowing is disabled (window = max): we only care about the
    // upper timeout (catching tasks that run too SLOW / hang), not refreshing too early.
    iwdg_h.Instance = IWDG;
    iwdg_h.Init.Prescaler = IWDG_PRESCALAR;
    iwdg_h.Init.Reload = IWDG_COUNTDOWN_TICKS;
    iwdg_h.Init.Window = IWDG_WINDOW_DISABLE;
}

void IWDG_Start() {
    // Initialize / start IWDG and check init status. (Post-reset detection happens earlier in
    // Init_WDogTask so a watchdog reset is reported before contactors can close.)
    if (HAL_IWDG_Init(&iwdg_h) != HAL_OK) {
        Error_Handler();
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