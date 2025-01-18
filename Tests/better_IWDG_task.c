/* Better Watchdog Task (with 1 dummy task)
 - Actual implementation of tasks from Task_PetWatchdog and Task_DummyTask (in folder Tasks/)
*/

#include "BPS_Tasks.h"

int main(void) {
   if (HAL_Init() != HAL_OK) {
      error_handler();
   }

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