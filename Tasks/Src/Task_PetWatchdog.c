/*-----------------------------------------------------------
 * PET WATCHDOG TASK
 - Attempts to pet watchdog within appropriate time interval
 -----------------------------------------------------------*/

// Kernel Includes
#include "FreeRTOS.h"   /* Must come first. */
#include "task.h"       /* RTOS task related API prototypes. */
#include "queue.h"      /* RTOS queue related API prototypes. */
#include "timers.h"     /* Software timer related API prototypes. */
#include "semphr.h"     /* Semaphore related API prototypes. */
#include "stm32xx_hal.h"

#include "IWDG.h"

// Task Parameters
// StaticTask_t Task_PetWD_Buffer;
// StackType_t Task_PetWD_Stack[configMINIMAL_STACK_SIZE];


SemaphoreHandle_t xIWDG_Semaphore = NULL;
StaticSemaphore_t xIWDG_SemaphoreBuffer;

/*-----------------------------------------------------------*/

/* TASK: Refreshes watchdog */
void Task_PetWatchdog() {
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };

    if(IWDG_CheckIfReset() == 1) {
        IWDG_Error_Handler();
    }
    
    // Set LED off to indicate we are in the init stage
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(500);

    IWDG_Init(led_init, IWDG_Error_Handler);

    // semaphore stuff 
    // xIWDG_Semaphore = xSemaphoreCreateMutexStatic(&xIWDG_SemaphoreBuffer);

    // refresh and toggle LED
    while(1) {
        // if(xIWDG_Semaphore != NULL) {
        //     if (xSemaphoreTake(xIWDG_Semaphore, portMAX_DELAY) == pdTRUE) {
        //         IWDG_Refresh();
        //         HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        //         xSemaphoreGive(xIWDG_Semaphore);
        //     }

        // }

        // if we receive notif from other task, refresh IWDG

        IWDG_Refresh();
        // HAL_Delay(SYS_REFRESH_MS);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

    }
}