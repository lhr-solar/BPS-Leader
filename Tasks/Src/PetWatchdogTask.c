/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/
#include "BPS_Tasks.h"
#include "WDog.h"
#include <timers.h>


/**
 * @brief Watchdog Event Group Initialization
 */
void Init_WDogEventGroup() {
    // Event Group init
    xWDogEventGroup_handle = xEventGroupCreateStatic( &xWDogEventGroup );
    configASSERT( xWDogEventGroup_handle );          // check if handle is set 
    xEventGroupClearBits(xWDogEventGroup_handle,     /* The event group being updated. */
                         0xFF );                /* The bits being cleared. */
}


/**
 * @brief Callback function for Watchdog Window Timer.
 *        Sets event group bit to signal Watchdog.
 */
void WDog_WindowCallback(TimerHandle_t xTimer) {
    xEventGroupSetBits(xWDogEventGroup_handle, TIMER_DONE);
}

/*--------------------------------------------------------*/

void Task_PETWDOG() {
    // Initialize GPIO output for Pins 5 (LED), 6 (Task 1), 7 (Task 2)
    GPIO_InitTypeDef gpio_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
    };

    WDog_Init(gpio_config, WDog_Error_Handler);

    /* RTOS Timer */
    // vTimerResetState();
    TimerHandle_t xWindowTimer;
    StaticTimer_t xTimerBuffer;

    xWindowTimer = xTimerCreateStatic( 
                    "Timer",                /* Just a text name, not used by the RTOS kernel. */
                    WDOG_WINDOW_MS,         /* The timer period in ticks????, must be greater than 0. */ 
                    pdFALSE,                /* Whether timer will auto-reload after expiring. */
                    NULL,                   /* ID assigned to timer being created */
                    WDog_WindowCallback,          /* Callback when timer expires */
                    &xTimerBuffer);    
    
    // Start timer; do not wait for timer to be sent to timer command queue
    xTimerStart(xWindowTimer, 0);
    
    while(1) {
        // Event group: wait until tasks + window timer are ready before refreshing
        uxBits = xEventGroupWaitBits(
                    xWDogEventGroup_handle, /* The event group being tested. */
                    ALL_TASKS_BITS,         /* The bits within the event group to wait for. */
                    pdFALSE,                /* Do not clear bits before returning. */
                    pdTRUE,                 /* Wait for both all bits to be set. */
                    portMAX_DELAY);         /* Maximum delay; block indefinitely */

        if((uxBits & ALL_TASKS_BITS) == ALL_TASKS_BITS) {
            // If we are here, xEventGroupWaitBits returned because bits were set

            // Window timer has run down; can refresh Watchdog
            WDog_Refresh();
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

            // Manually clear bits
            xEventGroupClearBits(xWDogEventGroup_handle, ALL_TASKS_BITS);
            // uxBits = xEventGroupGetBits(xEventGroupHandle);

            // Reset timer and do not wait for it to be sent to timer queue
            xTimerStart(xWindowTimer, 0);
        }

    }

}