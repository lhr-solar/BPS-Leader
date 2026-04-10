#include "common.h"
#include "BPS_Tasks.h"
#include "EMC2305_Driver.h"



void Task_Fan_Controller() {

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(FAN_CONTROLLER_TASK_DELAY_MS));

        // sets pwm of fans to 20
        set_fan_pwm(EMC2305_FAN1, 20);
        set_fan_pwm(EMC2305_FAN2, 20);

        
    }
}

