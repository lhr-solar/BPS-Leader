#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"


void Task_Contactor_Monitor(){

    while(1){

        // Delays 10 ms
        vTaskDelay(CONTACTOR_MONITOR_TASK_DELAY_MS);


        // Set event group bit
        // xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
        //                    TEMP_MONITOR_DONE);         /* The bits being set. */
    }
    
}