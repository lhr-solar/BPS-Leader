/* Better Watchdog Task (with 1 dummy task) */

#include "Task_PetWatchdog.h"

// Dummy Task 1
#define TASK_DUMMY_PRIORITY         tskIDLE_PRIORITY + 2
#define TASK_DUMMY_STACK_SIZE       configMINIMAL_STACK_SIZE
StackType_t Task_Dummy_Stack[configMINIMAL_STACK_SIZE];
StaticTask_t Task_Dummy_Buffer;


void Task_DummyTask() {
   GPIO_InitTypeDef gpio_init = {
      .Mode = GPIO_MODE_OUTPUT_PP,
      .Pull = GPIO_NOPULL,
      .Pin = GPIO_PIN_6 | GPIO_PIN_7
   };
   __HAL_RCC_GPIOA_CLK_ENABLE();
   HAL_GPIO_Init(GPIOA, &gpio_init);

   while(1) {
      if(xIWDG_Semaphore != NULL) {
         if (xSemaphoreTake(xIWDG_Semaphore, portMAX_DELAY) == pdTRUE) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
            xSemaphoreGive(xIWDG_Semaphore);

            HAL_Delay(SYS_REFRESH_MS);
         }
      }

      // xSemaphoreTake(xSemaphore, portTICK_PERIOD_MS * 50);
      // HAL_Delay(50);
      // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
      // xSemaphoreGive(xSemaphore);
   }
}

static void HAL_init_error_handler() {
   while(1){ 
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
      HAL_Delay(50);
   }
}


int main(void) {
   if (HAL_Init() != HAL_OK) {
      HAL_init_error_handler();
   }

   xIWDG_Semaphore = xSemaphoreCreateMutexStatic(&xIWDG_SemaphoreBuffer);

   xTaskCreateStatic(
                  Task_PetWatchdog,
                  "PetWatchdog",
                  TASK_PETWD_STACK_SIZE,
                  NULL,
                  TASK_PETWD_PRIORITY,
                  Task_PetWD_Stack,
                  &Task_PetWD_Buffer);

   xTaskCreateStatic(
                  Task_DummyTask,
                  "DummyTask1",
                  TASK_DUMMY_STACK_SIZE,
                  NULL,
                  TASK_DUMMY_PRIORITY,
                  Task_Dummy_Stack,
                  &Task_Dummy_Buffer);

   vTaskStartScheduler();

   return 0;
}