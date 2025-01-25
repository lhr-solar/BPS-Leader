/* Draft Watchdog Task with 2 dummy tasks
 - Attempts to pet watchdog while running two other tasks concurrently
 - Rough draft: everything is defined here
*/

// Kernel Includes
#include "FreeRTOS.h"   /* Must come first. */
#include "task.h"       /* RTOS task related API prototypes. */
#include "queue.h"      /* RTOS queue related API prototypes. */
#include "timers.h"     /* Software timer related API prototypes. */
#include "semphr.h"     /* Semaphore related API prototypes. */
#include "stm32xx_hal.h"

#include "IWDG.h"
#include <event_groups.h>

// Pet Watchdog Task
#define TASK_PETWDOG_PRIORITY     tskIDLE_PRIORITY + 1
#define TASK_PETWDOG_STACK_SIZE   configMINIMAL_STACK_SIZE
StaticTask_t Task_PetWDOG_Buffer;
StackType_t Task_PetWDOG_Stack[configMINIMAL_STACK_SIZE];

// Dummy Task 1
#define TASK_DUMMY_PRIORITY      tskIDLE_PRIORITY + 2
#define TASK_DUMMY_STACK_SIZE     configMINIMAL_STACK_SIZE
StaticTask_t dummy_task_buffer;
StackType_t dummy_taskStack[configMINIMAL_STACK_SIZE];

// Dummy Task 2
#define TASK_DUMMY2_PRIORITY     tskIDLE_PRIORITY + 3
#define TASK_DUMMY2_STACK_SIZE    configMINIMAL_STACK_SIZE
StaticTask_t dummy2_task_buffer;
StackType_t dummy2_taskStack[configMINIMAL_STACK_SIZE];

// task notification values
#define DUM1_DONE   0x01
#define DUM2_DONE   0x02

/*--------------------------------------------------------*/

static void GPIO_Init() {
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
      HAL_Delay(200);
   }
}

// static void success_handler(void) {
// }


/*-------------------------- TASKS ------------------------------*/

// TaskHandle_t xTask_PetWatchdog_Handle;


EventGroupHandle_t xEventGroupHandle;
StaticEventGroup_t xCreatedEventGroup;
EventBits_t uxBits;


/* TASK: Refreshes watchdog */
void m_Task_PetWatchdog(void *pvParameters) {
   // HAL_Init();
   // GPIO_InitTypeDef led_init = {
   //    .Mode = GPIO_MODE_OUTPUT_PP,
   //    .Pull = GPIO_NOPULL,
   //    .Pin = GPIO_PIN_5
   // };
   
   // if(IWDG_CheckIfReset() == 1) {
   //    error_handler();
   // }

   // Set LED off to indicate we are in the init stage
   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
   HAL_Delay(500);
   
   // IWDG_Init(led_init, IWDG_Error_Handler);

   // event group
   xEventGroupHandle = xEventGroupCreateStatic( &xCreatedEventGroup );
   configASSERT( xEventGroupHandle );  // check if handle is set 
   xEventGroupClearBits(xEventGroupHandle,   /* The event group being updated. */
                         0x0F );    /* The bits being cleared. */

   // Task notification
   // uint32_t notifValue;
   // const TickType_t xMaxBlockTime = portMAX_DELAY; // pdMS_TO_TICKS( 20 );
   // BaseType_t xResult;

   while(1) {
      // Task notification
      // xResult = xTaskNotifyWait( pdFALSE,          /* Don't clear bits on entry. */
      //                            0x6,              /* Clear bits 1,2 on exit */
      //                            &notifValue,      /* Stores the notified value. */
      //                            xMaxBlockTime );
      // if(xResult == pdPASS) {
      //    if((notifValue & DUM1_DONE) && (notifValue & DUM2_DONE)) {
      //       IWDG_Refresh();
      //       HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      //    }
      // }
      

      // event group
      // const TickType_t xTicksToWait = 20 / portTICK_PERIOD_MS;
   
      uxBits = xEventGroupWaitBits(
               xEventGroupHandle,            /* The event group being tested. */
               DUM1_DONE | DUM2_DONE,        /* The bits within the event group to wait for. */
               pdTRUE,                       /* BIT_0 & BIT_4 should be cleared before returning. */
               pdTRUE,                       /* Don't wait for both bits, either bit will do. */
               portMAX_DELAY);               /* Wait a maximum of 100ms for either bit to be set. */

    if((uxBits & (DUM1_DONE | DUM2_DONE)) == (DUM1_DONE | DUM2_DONE)) {
      // xEventGroupWaitBits returned because bits 1,2 were set
      // IWDG_Refresh();
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }


      // IWDG_Refresh();
      // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      // HAL_Delay(SYS_REFRESH_MS);
   }
}

/* TASK: Toggles Pin A6 */
static void dummy_task(void *pvParameters) {
   while(1) {
      // event group 
      xEventGroupSetBits(xEventGroupHandle,     /* The event group being updated. */
                                 DUM1_DONE);             /* The bits being set. */
      // task notification
      // xTaskNotify(xTask_PetWatchdog_Handle, DUM1_DONE, eSetBits); 

      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
      HAL_Delay(10);
   }
}

/* TASK: Toggles Pin A7 */
static void dummy_task_two(void *pvParameters) {
   while(1) {
      // event group 
      xEventGroupSetBits(xEventGroupHandle,     /* The event group being updated. */
                                 DUM2_DONE);             /* The bits being set. */
      // task notification
      // xTaskNotify(xTask_PetWatchdog_Handle, DUM2_DONE, eSetBits);
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
      HAL_Delay(5);
   }
}

/*-----------------------------------------------------------*/

int main(void) {
   if (HAL_Init() != HAL_OK) {
      error_handler();
   }

   GPIO_Init();

   // xTask_PetWatchdog_Handle = 
   xTaskCreateStatic(
                  m_Task_PetWatchdog,
                  "PetWatchdog",
                  TASK_PETWDOG_STACK_SIZE,
                  NULL,
                  TASK_PETWDOG_PRIORITY,
                  Task_PetWDOG_Stack,
                  &Task_PetWDOG_Buffer);

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