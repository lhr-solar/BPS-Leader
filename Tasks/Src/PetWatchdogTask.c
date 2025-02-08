/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/
#include "BPS_Tasks.h"
#include "WDog.h"


void Task_PETWDOG() {
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };

    WDog_Init(led_init, WDog_Error_Handler);

    // const TickType_t xTicksToWait = 5 / portTICK_PERIOD_MS;

    while(1) {
        // Event group: wait until dummy task 1 and 2 have run before refreshing Watchdog
        uxBits = xEventGroupWaitBits(
                    xEventGroupHandle,      /* The event group being tested. */
                    DUM1_DONE | DUM2_DONE,  /* The bits within the event group to wait for. */
                    pdTRUE,                 /* Clear bits before returning. */
                    pdTRUE,                 /* Don't wait for both bits, either bit will do. */
                    portMAX_DELAY);         /* Maximum delay; block indefinitely?  */

        if((uxBits & (DUM1_DONE | DUM2_DONE)) == (DUM1_DONE | DUM2_DONE)) {
            // xEventGroupWaitBits returned because bits 1,2 were set
            
            WDog_Refresh();
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
        xEventGroupClearBits(xEventGroupHandle, (DUM1_DONE | DUM2_DONE));   // Manually clear bits before next loop
    }
 }