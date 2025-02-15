/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/
#include "BPS_Tasks.h"
#include "WDog.h"
#include <timers.h>

void Init_WDogEventGroup() {
    // Event Group init
    xEventGroupHandle = xEventGroupCreateStatic( &xCreatedEventGroup );
    configASSERT( xEventGroupHandle );          // check if handle is set 
    xEventGroupClearBits(xEventGroupHandle,     /* The event group being updated. */
                         0xFF );                /* The bits being cleared. */
}

void Task_PETWDOG() {
    // Initialize GPIO output for Pins 5 (LED), 6 (Task 1), 7 (Task 2)
    GPIO_InitTypeDef gpio_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
    };
    // __HAL_RCC_GPIOA_CLK_ENABLE(); 
	// HAL_GPIO_Init(GPIOA, &gpio_config);

    WDog_Init(gpio_config, WDog_Error_Handler);

    /* RTOS Timer */
    // vTimerResetState();
    TimerHandle_t xWindowTimer;
    StaticTimer_t xTimerBuffer;

    xWindowTimer = xTimerCreateStatic( 
                    "Timer",            /* Just a text name, not used by the RTOS kernel. */
                    5,   /* The timer period in ticks, must be greater than 0. */ 
                    pdFALSE,            /* Whether timer will auto-reload after expiring. */
                    NULL,               /* ID assigned to timer being created */
                    NULL,               /* Callback when timer expires */
                    &xTimerBuffer);     
    
    // Start timer; wait up to 1ms for timer to be sent to timer command queue
    xTimerStart(xWindowTimer, 1);  // pdMS_TO_TICKS(1)

    // const TickType_t xTicksToWait = portMAX_DELAY;  // (5 / portTICK_PERIOD_MS)

    while(1) {
        // Event group: wait until Tasks 1 and 2 are done before refreshing Watchdog
        uxBits = xEventGroupWaitBits(
                    xEventGroupHandle,      /* The event group being tested. */
                    TASK1_BIT | TASK2_BIT,  /* The bits within the event group to wait for. */
                    pdFALSE,                /* Do not clear bits before returning. */
                    pdTRUE,                 /* Wait for both all bits to be set. */
                    portMAX_DELAY);         /* Maximum delay; block indefinitely?  */

        while((uxBits & ALL_TASKS_BITS) == ALL_TASKS_BITS) {
            // If we are here, xEventGroupWaitBits returned because bits were set
            
            // window for refresh
            if(xTimerIsTimerActive(xWindowTimer) == pdFALSE) {
                // Window timer has run down; can now refresh Watchdog
                WDog_Refresh();
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

                // Manually clear bits
                xEventGroupClearBits(xEventGroupHandle, ALL_TASKS_BITS);
                // uxBits = xEventGroupGetBits(xEventGroupHandle);

                // Reset timer
                xTimerReset(xWindowTimer, 1);   
                break;
            }
            vTaskDelay(1);
            
        }
    }

}