/* Pet IWDG based on running tasks
 - Attempts to pet watchdog while running two other tasks concurrently
*/

#include "BPS_Tasks.h"
#include "IWDG.h"


// Dummy Task 1
#define TASK_DUMMY_PRIORITY      tskIDLE_PRIORITY + 3
#define TASK_DUMMY_STACK_SIZE     configMINIMAL_STACK_SIZE
StaticTask_t dummy_task_buffer;
StackType_t dummy_taskStack[configMINIMAL_STACK_SIZE];

// Dummy Task 2
#define TASK_DUMMY2_PRIORITY     tskIDLE_PRIORITY + 4
#define TASK_DUMMY2_STACK_SIZE    configMINIMAL_STACK_SIZE
StaticTask_t dummy2_task_buffer;
StackType_t dummy2_taskStack[configMINIMAL_STACK_SIZE];

/*--------------------------------------------------------*/

static void GPIO_Init() {
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = (GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
    };

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &led_init);
}


/* Blinks LED to signal we have faulted */
static void error_handler(void) {
    // vTaskEndScheduler();
    while(1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(200);
    }
}

/*-------------------------- TASKS ------------------------------*/


/* TASK: Toggles Pin A6 */
static void dummy_task(void *pvParameters) {
    while(1) {
        xEventGroupSetBits(xWDogEventGroup_handle, /* The event group being updated. */
                                    VOLT_MONITOR_DONE);    /* The bits being set. */
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
        vTaskDelay(10);
    }
}

/* TASK: Toggles Pin A7 */
static void dummy_task_two(void *pvParameters) {
    while(1) {
        xEventGroupSetBits(xWDogEventGroup_handle, /* The event group being updated. */
                                    TEMP_MONITOR_DONE);    /* The bits being set. */
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
        vTaskDelay(5);
    }
}

/*-----------------------------------------------------------*/

int main(void) {
    if (HAL_Init() != HAL_OK) {
        error_handler();
    }
    
    SystemClock_Config();
    GPIO_Init();

    Init_WDogTask();

    xTaskCreateStatic(
                Task_PetWatchdog,
                "PetWatchdog",
                TASK_PETWDOG_STACK_SIZE,
                NULL,
                TASK_PETWDOG_PRIO,
                Task_Petwdog_Stack_Array,
                &Task_Petwdog_Buffer);

    xTaskCreateStatic(
                dummy_task,
                "DummyTask1",
                TASK_DUMMY_STACK_SIZE,
                NULL,
                TASK_DUMMY_PRIORITY,
                dummy_taskStack,
                &dummy_task_buffer);

    xTaskCreateStatic(
                dummy_task_two,
                "DummyTask2",
                TASK_DUMMY2_STACK_SIZE,
                NULL,
                TASK_DUMMY2_PRIORITY,
                dummy2_taskStack,
                &dummy2_task_buffer);

    vTaskStartScheduler();

   return 0;
}