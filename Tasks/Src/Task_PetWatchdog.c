/* Pet Watchdog Task
 - Attempts to pet watchdog within appropriate time interval
*/

#include "BPS_Tasks.h"
#include "IWDG.h"

/*-----------------------------------------------------------*/

static void GPIO_Init() {
   /* LED: GPIO A, Pin 5*/
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };

   __HAL_RCC_GPIOA_CLK_ENABLE();
   HAL_GPIO_Init(GPIOA, &led_init);
}

StaticSemaphore_t xSemaphoreBuffer;
/*-----------------------------------------------------------*/

/* TASK: Refreshes watchdog */
void Task_PetWatchdog() {
    GPIO_Init();
    
    // TO-DO: Don't fault on first run through
    if(IWDG_CheckIfReset() == 1) {
        error_handler();
    }

    // Set LED off to indicate we are in the init stage
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(500);
    IWDG_Init();

    // semaphore stuff (i don't really know what i'm doing
    static SemaphoreHandle_t xEventSemaphore = NULL;
    xEventSemaphore = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);

    // refresh and toggle LED
    while(1) {
        // take
        xSemaphoreTake(xEventSemaphore, portMAX_DELAY);

        IWDG_Refresh();
        // HAL_Delay(SYS_REFRESH_MS);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

        // give mutex
        xSemaphoreGive(xEventSemaphore);
    }
}