/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/
#include "BPS_Tasks.h"
#include "WDog.h"
#include <timers.h>

void WDog_InitEventGroup() {
    // Event Group init
    xEventGroupHandle = xEventGroupCreateStatic( &xCreatedEventGroup );
    configASSERT( xEventGroupHandle );          // check if handle is set 
    xEventGroupClearBits(xEventGroupHandle,     /* The event group being updated. */
                         0x0F );                /* The bits being cleared. */
}

void Task_PETWDOG() {
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };

    WDog_Init(led_init, WDog_Error_Handler);

    /* RTOS Timer */
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

    // const TickType_t xTicksToWait = portMAX_DELAY;  // (5 / portTICK_PERIOD_MS)

    while(1) {
        // Event group: wait until dummy task 1 and 2 have run before refreshing Watchdog
        uxBits = xEventGroupWaitBits(
                    xEventGroupHandle,      /* The event group being tested. */
                    TASK1_BIT | TASK2_BIT,  /* The bits within the event group to wait for. */
                    pdFALSE,                /* Do not clear bits before returning. */
                    pdTRUE,                 /* Wait for both all bits to be set. */
                    portMAX_DELAY);         /* Maximum delay; block indefinitely?  */

        if((uxBits & ALL_TASKS_BITS) == ALL_TASKS_BITS) {
            // If we are here, xEventGroupWaitBits returned because bits were set
            
            // window for refresh
            // if(xTimerIsTimerActive(xWindowTimer) == pdFALSE) {
                // Window timer has run down; can now refresh Watchdog
                WDog_Refresh();               
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

                // Reset timer
                // xTimerReset(xWindowTimer, 50);   

                // Manually clear bits
                xEventGroupClearBits(xEventGroupHandle, ALL_TASKS_BITS);   
            // }
            
        }
    }
 }