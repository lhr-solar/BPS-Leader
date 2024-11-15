#include "BPS_Tasks.h"
#include "LEDs.h"

void Task_Init(){
    //Heartbeat_Init();
    //Heartbeat_Toggle();

    // Task deletes itself when all other tasks are Init'd
    vTaskDelete(NULL);

    
}