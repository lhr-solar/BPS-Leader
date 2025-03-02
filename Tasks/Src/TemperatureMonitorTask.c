#include "BPS_Tasks.h"

void Task_Temperature_Monitor(){

    while(1){
        // Delays 10 ms
        vTaskDelay(TEMP_MONITOR_TASK_DELAY);

        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
                           TEMP_MONITOR_DONE);         /* The bits being set. */
    }
    
}