#include "BPS_Tasks.h"

void Task_Temperature_Monitor(){

    while(1){
        // Delays 10 ms
        vTaskDelay(TEMP_MONITOR_TASK_DELAY);

        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle,   /* The event group being updated. */
                           TASK1_BIT);          /* The bits being set. */

        // Toggle Pin for debug
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
    }
    
}