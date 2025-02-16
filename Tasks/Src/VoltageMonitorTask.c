#include "BPS_Tasks.h"

void Task_Voltage_Monitor(){

    while(1){
        // Delays 5 ms
        vTaskDelay(VOLT_MONITOR_TASK_DELAY);
        
        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle,   /* The event group being updated. */
                           TASK2_BIT);          /* The bits being set. */

        #ifdef DEBUG
        // Toggle Pin for debug
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
        #endif
    }
    
}