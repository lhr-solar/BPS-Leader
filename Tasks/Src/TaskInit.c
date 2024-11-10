#include "Tasks.h"

void Task_Init(){

    // Task deletes itself when all other tasks are Init'd
    vTaskDelete(NULL);
    
    
}