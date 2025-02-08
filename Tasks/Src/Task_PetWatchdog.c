/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/
#include "Task_PetWatchdog.h"

// Task
StaticTask_t Task_PetWD_Buffer;
StackType_t Task_PetWD_Stack[configMINIMAL_STACK_SIZE];

// Event group
EventGroupHandle_t xEventGroupHandle;
StaticEventGroup_t xCreatedEventGroup;
EventBits_t uxBits;
/*-----------------------------------------------------------*/

void Task_PetWatchdog(void *pvParameters) {
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };

    // Set LED off to indicate we are in the init stage
    // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    // HAL_Delay(500);

    IWDG_Init(led_init, IWDG_Error_Handler);

    const TickType_t xTicksToWait = 5 / portTICK_PERIOD_MS;

    while(1) {
        // Event group: wait until dummy task 1 and 2 have run before refreshing Watchdog
        uxBits = xEventGroupWaitBits(
                    xEventGroupHandle,      /* The event group being tested. */
                    DUM1_DONE | DUM2_DONE,  /* The bits within the event group to wait for. */
                    pdTRUE,                 /* BIT_0 & BIT_4 should be cleared before returning. */
                    pdTRUE,                 /* Don't wait for both bits, either bit will do. */
                    xTicksToWait);          /* Wait a maximum of 100ms for either bit to be set. */

        if((uxBits & (DUM1_DONE | DUM2_DONE)) == (DUM1_DONE | DUM2_DONE)) {
            // xEventGroupWaitBits returned because bits 1,2 were set
            IWDG_Refresh();
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
    }
 }