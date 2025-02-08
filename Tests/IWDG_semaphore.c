/* Better Watchdog Task (with 1 dummy task) */


// DOES NOT WORK I"M TELLING YOU


#include "FreeRTOS.h"   /* Must come first. */
#include "task.h"       /* RTOS task related API prototypes. */
#include "queue.h"      /* RTOS queue related API prototypes. */
#include "timers.h"     /* Software timer related API prototypes. */
#include "semphr.h"     /* Semaphore related API prototypes. */

#include "stm32f4xx.h"
#include "IWDG.h"

// Watchdog Task
#define TASK_M_PETWD_PRIORITY          tskIDLE_PRIORITY + 1
#define TASK_M_PETWD_STACK_SIZE        configMINIMAL_STACK_SIZE
StackType_t Task_m_PetWD_Stack[configMINIMAL_STACK_SIZE];
StaticTask_t Task_m_PetWD_Buffer;

// Dummy Task 1
#define TASK_DUMMY_PRIORITY            tskIDLE_PRIORITY + 1
#define TASK_DUMMY_STACK_SIZE          configMINIMAL_STACK_SIZE
StackType_t Task_Dummy_Stack[configMINIMAL_STACK_SIZE];
StaticTask_t Task_Dummy_Buffer;

// Semaphore (old)
SemaphoreHandle_t xIWDG_Semaphore = NULL;
StaticSemaphore_t xIWDG_SemaphoreBuffer;
uint8_t Flags = 0x0;
uint8_t TestBit = 0x3;

/* TASK: Refreshes watchdog */
void m_Task_PetWatchdog() {
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    
    // Set LED off to indicate we are in the init stage
   //  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
   //  HAL_Delay(500);

    IWDG_Init(led_init, IWDG_Error_Handler);

    // refresh and toggle LED
    while(1) {
        if((xIWDG_Semaphore != NULL) && (xSemaphoreTake(xIWDG_Semaphore, portMAX_DELAY) == pdTRUE)) {
            if((Flags & TestBit) == TestBit) {
               IWDG_Refresh();
               HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
               Flags &= 0x0;
            }
            xSemaphoreGive(xIWDG_Semaphore);
      }
   }
}

void Task_DummyTask() {
   GPIO_InitTypeDef gpio_init = {
      .Mode = GPIO_MODE_OUTPUT_PP,
      .Pull = GPIO_NOPULL,
      .Pin = GPIO_PIN_6
   };
   __HAL_RCC_GPIOA_CLK_ENABLE();
   HAL_GPIO_Init(GPIOA, &gpio_init);

   while(1) {
      if((xIWDG_Semaphore != NULL) && (xSemaphoreTake(xIWDG_Semaphore, portMAX_DELAY) == pdTRUE)) {
         HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
         Flags |= TestBit;
         xSemaphoreGive(xIWDG_Semaphore);
         HAL_Delay(15);
         }
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
                  m_Task_PetWatchdog,
                  "PetWatchdog",
                  TASK_M_PETWD_STACK_SIZE,
                  NULL,
                  TASK_M_PETWD_PRIORITY,
                  Task_m_PetWD_Stack,
                  &Task_m_PetWD_Buffer);

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