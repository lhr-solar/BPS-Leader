/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/
#include "BPS_Tasks.h"
#include "WDog.h"
#include <timers.h>


void Task_PETWDOG() {
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };

    WDog_Init(led_init, WDog_Error_Handler);
    // const TickType_t xTicksToWait = 5 / portTICK_PERIOD_MS;

    // vTimerResetState();
    // TimerHandle_t xWindowTimer;
    // StaticTimer_t xTimerBuffer;

    // xWindowTimer = xTimerCreateStatic( 
    //                 "Timer",            /* Just a text name, not used by the RTOS kernel. */
    // // pdMS_TO_TICKS(WDOG_TIMEOUT_MS / 5)
    //                 1,   /* The timer period in ticks, must be greater than 0. */ 
    //                 pdFALSE,            /* Whether timer will auto-reload after expiring. */
    //                 NULL,               /* ID assigned to timer being created */
    //                 NULL,               /* Callback when timer expires */
    //                 &xTimerBuffer);     
    
    // xTimerStart(xWindowTimer, 50);

    while(1) {
        // Event group: wait until dummy task 1 and 2 have run before refreshing Watchdog
        uxBits = xEventGroupWaitBits(
                    xEventGroupHandle,      /* The event group being tested. */
                    DUM1_DONE | DUM2_DONE,  /* The bits within the event group to wait for. */
                    pdFALSE,                 /* Clear bits before returning. */
                    pdTRUE,                 /* Don't wait for both bits, either bit will do. */
                    portMAX_DELAY);         /* Maximum delay; block indefinitely?  */

        if((uxBits & (DUM1_DONE | DUM2_DONE)) == (DUM1_DONE | DUM2_DONE)) {
            // If we are here, xEventGroupWaitBits returned because bits were set
            
            // window for refresh
            // if(xTimerIsTimerActive(xWindowTimer) == pdFALSE) {
                WDog_Refresh();                 // Window timer has run down; can now refresh Watchdog
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

                // xTimerReset(xWindowTimer, 50);   // Reset timer
                xEventGroupClearBits(xEventGroupHandle, (DUM1_DONE | DUM2_DONE));   // Manually clear bits
            // }
            
        }
    }
 }