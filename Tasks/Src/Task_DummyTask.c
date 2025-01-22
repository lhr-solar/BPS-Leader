/*-----------------------------------------------------------
 * DUMMY TASK
 - Toggles output to GPIO Pin A6
/*-----------------------------------------------------------*/
#include "BPS_Tasks.h"

static void GPIO_Init() {
    GPIO_InitTypeDef init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin =  GPIO_PIN_6
    };

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &init);
}


/* TASK: Toggles Pin A6 */
void Task_DummyTask() {
    GPIO_Init();
    while(1) {
        xSemaphoreTake(xEventSemaphore, 0);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
        HAL_Delay(8);
        xSemaphoreGive(xEventSemaphore);
    }
}
