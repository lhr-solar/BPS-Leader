/* Watchdog Task
 - Attempts to pet watchdog within appropriate time interval
*/

#include "FreeRTOS.h"
#include "task.h"
#include "IWDG.h"
#include "stm32xx_hal.h"

// Task Priorities
#define TASK_PETWD_PRIORITY      tskIDLE_PRIORITY + 5
#define TASK_DUMMY_PRIORITY      tskIDLE_PRIORITY + 5
#define TASK_DUMMY2_PRIORITY     tskIDLE_PRIORITY + 5

// Task Stack Sizes
#define TASK_PETWD_STACK_SIZE     configMINIMAL_STACK_SIZE
#define TASK_DUMMY_STACK_SIZE     configMINIMAL_STACK_SIZE
#define TASK_DUMMY2_STACK_SIZE    configMINIMAL_STACK_SIZE

// Task Buffers
StaticTask_t petWD_task_buffer;
StaticTask_t dummy_task_buffer;
StaticTask_t dummy2_task_buffer;

// Task Stack Arrays
StackType_t petWD_taskStack[configMINIMAL_STACK_SIZE];
StackType_t dummy_taskStack[configMINIMAL_STACK_SIZE];
StackType_t dummy2_taskStack[configMINIMAL_STACK_SIZE];


static void GPIO_Init() {
   /* LED: GPIO A, Pin 5*/
   GPIO_InitTypeDef led_init = {
   .Mode = GPIO_MODE_OUTPUT_PP,
   .Pull = GPIO_NOPULL,
   .Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
   };

   __HAL_RCC_GPIOA_CLK_ENABLE();
   HAL_GPIO_Init(GPIOA, &led_init);
}


/* Blinks LED to signal we have faulted */
static void error_handler(void) {
   // vTaskEndScheduler();
   while(1) {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(100);
   }
}

// static void success_handler(void) {
// }


/*-------------------------- TASKS ------------------------------*/


/* TASK: Refreshes watchdog */
static void Task_PetWatchdog(void *pvParameters) {
   // HAL_Init();
   
   if(IWDG_CheckIfReset() == 1) {
      error_handler();
   }

   // Set LED off to indicate we are in the init stage
   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
   HAL_Delay(500);
   IWDG_Init();

   // refresh within time limit for 50 cycles, then force watchdog to trip
   while(1) {
      static int i = 1;
      IWDG_Refresh();
      i > 50 ? HAL_Delay(20) : HAL_Delay(SYS_REFRESH_MS);

      // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      i++;
   }
}

/* TASK: Toggles Pin A6 */
static void dummy_task(void *pvParameters) {
   while(1) {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
      HAL_Delay(500);
   }
}

/* TASK: Toggles Pin A7 */
static void dummy_task_two(void *pvParameters) {
   while(1) {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
      HAL_Delay(250);
   }
}

/*-----------------------------------------------------------*/

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
                  &petWD_task_buffer);

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