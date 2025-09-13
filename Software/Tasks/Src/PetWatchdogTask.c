/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/
#include "BPS_Tasks.h"
#include "IWDG.h"
#include "LEDs.h"
#include <timers.h>


/* RTOS Timer - Used as a window period for IWDG */
TimerHandle_t xWindowTimer;
StaticTimer_t xTimerBuffer;

/* Event group */
StaticEventGroup_t xWDogEventGroup;
EventBits_t uxBits;


/**
 * @brief Callback function for Watchdog Window Timer.
 *        Sets event group bit to signal Watchdog.
 */
void WDog_WindowCallback(TimerHandle_t xTimer) {
    xEventGroupSetBits(xWDogEventGroup_handle, WINDOW_TIMER_DONE);
}

/**
 * @brief Watchdog Task Initialization
 *        (1) Inits the event group
 *        (2) Inits the window timer
 *        (3) Inits IWDG but does not start it
 */
void Init_WDogTask() {
    // Event Group init
    xWDogEventGroup_handle = xEventGroupCreateStatic( &xWDogEventGroup );
    configASSERT( xWDogEventGroup_handle );         // check if handle is set 
    xEventGroupClearBits(xWDogEventGroup_handle,    /* The event group being updated. */
                         0xFF );                    /* The bits being cleared. */

    // Window timer init
    xWindowTimer = xTimerCreateStatic( 
                "Timer",                /* Just a text name, not used by the RTOS kernel. */
                IWDG_WINDOW_MS,         /* The timer period in ms (must be > 0). */ 
                pdFALSE,                /* Whether timer will auto-reload after expiring. */
                NULL,                   /* ID assigned to timer being created. */
                WDog_WindowCallback,    /* Callback when timer expires. */
                &xTimerBuffer);  
    
    // Inits IWDG but does not start
    IWDG_Init();
}

/*--------------------------------------------------------*/

void Task_PetWatchdog() {
    // Start timer; do not wait for timer to be sent to timer command queue
    xTimerStart(xWindowTimer, 0);
    
    IWDG_Start(IWDG_Error_Handler);

    while(1) {
        // Event group: wait until tasks + window timer are ready before refreshing
        uxBits = xEventGroupWaitBits(
                    xWDogEventGroup_handle, /* The event group being tested. */
                    ALL_TASKS_DONE,         /* The bits within the event group to wait for. */
                    pdFALSE,                /* Do not clear bits before returning. */
                    pdTRUE,                 /* Wait for both all bits to be set. */
                    portMAX_DELAY);         /* Maximum delay; block indefinitely */

        if((uxBits & ALL_TASKS_DONE) == ALL_TASKS_DONE) {
            /** 
             * If we are here, xEventGroupWaitBits returned because bits were set
             * and window timer has run down; can refresh Watchdog.
             */
            IWDG_Refresh();
            Heartbeat_Toggle();
            xEventGroupClearBits(xWDogEventGroup_handle, ALL_TASKS_DONE);

            // Reset timer and do not wait for it to be sent to timer queue
            xTimerStart(xWindowTimer, 0);
        }

    }

}