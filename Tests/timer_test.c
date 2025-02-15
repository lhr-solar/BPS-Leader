/* 
 Timer Test
 - Making sure RTOS timer works properly :(
*/
#include "BPS_Tasks.h"

// Task
#define TASK_TIMER_PRIO             tskIDLE_PRIORITY + 1
#define TASK_TIMER_STACK_SIZE       configMINIMAL_STACK_SIZE
StaticTask_t task_timer_buffer;
StackType_t task_timer_stack[configMINIMAL_STACK_SIZE];

// Timer
TimerHandle_t xWindowTimer;
StaticTimer_t xTimerBuffer;

void timerTest() {
    GPIO_InitTypeDef gpio_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    __HAL_RCC_GPIOA_CLK_ENABLE(); 
	HAL_GPIO_Init(GPIOA, &gpio_config);

    /* RTOS Timer */
    vTimerResetState();
    TickType_t timeout = 10 * 80000;

    // One shot timer
    xWindowTimer = xTimerCreateStatic( 
                    "Timer",            /* Just a text name, not used by the RTOS kernel. */
                    timeout,            /* The timer period in ticks, must be greater than 0. */ 
                    pdFALSE,            /* Whether timer will auto-reload after expiring. */
                    (void*)0,           /* ID assigned to timer being created */
                    (void*)0,           /* Callback when timer expires */
                    &xTimerBuffer);     
    
    // Start timer; wait up to 5 ticks for timer to be sent to timer command queue
    if(xTimerStart(xWindowTimer, 5) == pdPASS) {
        while(1) {
                
            // Toggle pin only when timer has run down
            if(xTimerIsTimerActive(xWindowTimer) == pdFALSE) {
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

                // Reset timer
                xTimerReset(xWindowTimer, 0);
            }
            else {
                vTaskDelay(1);
            }
            
        }
    }

}

int main(void) {
    if (HAL_Init() != HAL_OK) {
        while(1) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            HAL_Delay(200);
        }
    }

    xTaskCreateStatic(
                timerTest,
                "Timer Test",
                TASK_TIMER_STACK_SIZE,
                NULL,
                TASK_TIMER_PRIO,
                task_timer_stack,
                &task_timer_buffer);

    vTaskStartScheduler();

   return 0;
}