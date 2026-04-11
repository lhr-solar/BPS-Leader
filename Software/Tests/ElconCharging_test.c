
#include "common.h"
#include "StatusLEDs.h"
#include "Contactors.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "SHT45.h"
#include "EMC2305_Driver.h"
#include "DebugPrintf.h"
#include "BPS_Tasks.h"


int main(void) {
    HAL_Init();
    SystemClock_Config();

    LEDs_init();
    debugPrintf_init();

    if (CAN_Init() != CAN_OK) {
        printf("CAN_Init failed!\r\n");
        while (1);
    }

    xTaskCreateStatic(
        Task_Elcon_Charging,
        "Elcon Charging Task",
        TASK_ELCON_CHARGING_STACK_SIZE,
        (void*)NULL,
        TASK_ELCON_CHARGING_PRIO,
        Task_Elcon_Charging_Stack,
        &Task_Elcon_Charging_Buffer
    );

    xTaskCreateStatic(
        Task_FaultHandler,
        "Fault Handler Task",
        FAULT_HANDLER_TASK_STACK_SIZE,
        (void*)NULL,
        TASK_FAULT_HANDLER_PRIO,
        FaultHandler_Task_Stack,
        &FaultHandler_Task_Buffer
    );


    
    vTaskStartScheduler();

    while (1) {


    }

    return 0;
}