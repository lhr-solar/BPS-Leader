#include "BPS_Tasks.h"
#include "CAN.h"

/*--------------------------------------------------------*/
void Task_Amperes_Monitor(){

    while(1){
        // Delays
        vTaskDelay(AMPS_MONITOR_TASK_DELAY);

        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
                           AMPS_MONITOR_DONE);         /* The bits being set. */
    }
    
}