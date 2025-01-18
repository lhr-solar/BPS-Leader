// /* Watchdog Task
//  - Attempts to pet watchdog within appropriate time interval
// */

#include "FreeRTOS.h"
#include "task.h"
#include "IWDG.h"
#include "stm32xx_hal.h"

// Task Priorities
#define TASK_PETWD_PRIORITY      tskIDLE_PRIORITY + 5
#define TASK_DUMMY_PRIORITY      tskIDLE_PRIORITY + 5

// Task Stack Sizes
#define TASK_PETWD_STACK_SIZE     configMINIMAL_STACK_SIZE
#define TASK_DUMMY_STACK_SIZE     configMINIMAL_STACK_SIZE

// Task Buffers
StaticTask_t petWD_task_buffer;
StaticTask_t dummy_task_buffer;

// Task Stack Arrays
StackType_t petWD_taskStack[configMINIMAL_STACK_SIZE];
StackType_t dummy_taskStack[configMINIMAL_STACK_SIZE];


static void GPIO_Init() {
   /* LED: GPIO A, Pin 5*/
   GPIO_InitTypeDef led_init = {
   .Mode = GPIO_MODE_OUTPUT_PP,
   .Pull = GPIO_NOPULL,
   .Pin = GPIO_PIN_5| GPIO_PIN_6
   };

   __HAL_RCC_GPIOA_CLK_ENABLE();
   HAL_GPIO_Init(GPIOA, &led_init);
}

static void error_handler(void) {
   while(1){
      // Blinky LED to indicate we have faulted
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(100);
   }
}

// static void success_handler(void) {
// }

/* ===================== TASKS ===================== */
static void Task_PetWatchdog(void *pvParameters) {
   // HAL_Init();

   if(IWDG_CheckIfReset() == 1) {
         // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
         // HAL_Delay(500);
         error_handler();
   }

   // Set LED off to indicate we are in the init stage
   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
   HAL_Delay(500);
   IWDG_Init();

   while(1) {
      IWDG_Refresh();
      HAL_Delay(8);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
   }
}

static void dummy_task(void *pvParameters) {
   while(1) {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
      HAL_Delay(500);
   }
}

int main(void) {
   if (HAL_Init() != HAL_OK) {
      error_handler();
   }

   GPIO_Init();

   xTaskCreateStatic(
                  Task_PetWatchdog,
                  "PetWatchdog",
                  TASK_PETWD_STACK_SIZE,
                  NULL,
                  TASK_PETWD_PRIORITY,
                  petWD_taskStack,
                  &petWD_task_buffer
   );

   xTaskCreateStatic(
                  dummy_task,
                  "DummyTask1",
                  TASK_DUMMY_STACK_SIZE,
                  NULL,
                  TASK_DUMMY_PRIORITY,
                  dummy_taskStack,
                  &dummy_task_buffer
   );

   vTaskStartScheduler();

   return 0;
}