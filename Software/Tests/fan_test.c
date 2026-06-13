#include "common.h"
#include "BPS_Tasks.h"
#include "EMC2305_Driver.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "Contactors.h"
#include "DebugPrintf.h"
#include "SHT45.h"

#define STARTUP_DELAY_MS (500)

extern EMC2305_HandleTypeDef chip;

// Static task buffers
static StaticTask_t xTestTaskBuffer;
static StackType_t xTestStack[TEST_TASK_STACK_SIZE];

void vFanChipTestTask(void *pvParameters) {

    vTaskDelay(pdMS_TO_TICKS(STARTUP_DELAY_MS));

    if (faultHandler_init() == 0)
    {
        Error_Handler();
    }

    xStateBits = xEventGroupCreateStatic(&xStateBits_buffer);

    if (xStateBits == NULL) {
        Error_Handler();
    }

    LEDs_init();

    debugPrintf_init();

    CAN_Init();

    contactor_init();

    SHT45_init();

    EMC2305_I2C_init();
    if (EMC2305_Driver_init() != EMC2305_OK) printf("Fan Chip init error!");

    Init_WDogTask();

    printf("Initialized...\n\r");

    xTaskCreateStatic(
        Task_FaultHandler,             // Task function
        "FaultHandler",                // Name of the task (for debugging)
        FAULT_HANDLER_TASK_STACK_SIZE, // Stack size in words
        (void *)NULL,                  // Task input parameter
        TASK_FAULT_HANDLER_PRIO,       // Task priority
        FaultHandler_Task_Stack,       // Task handle
        &FaultHandler_Task_Buffer      // Static task buffer (optional)
    );

    while (true) {

        printf("\n\r\nNow Ramping to: FAN MIN RPM\n\r\n");

        if (set_fan_rpm(EMC2305_FAN2, EMC2305_MIN_RPM) != EMC2305_OK) {
            set_faultBit(FAN_CHIP_ERROR);
            printf("Error while ramping to min RPM\r\n");
            vTaskDelete(NULL);
        }; 
        for (uint16_t i = 0; i < 30; i++) {
            printf("Fan RPM: %u\r\n", EMC2305_GetFanRPM(&chip, EMC2305_FAN2));
            vTaskDelay(pdMS_TO_TICKS(500));
        }   

        // vTaskDelay(pdMS_TO_TICKS(2000));
        // printf("\n\r\nNow Ramping to: FAN MAX RPM\n\r\n");

        // if (EMC2305_SetFanRPM(&chip, EMC2305_FAN2, EMC2305_MAX_RPM) != EMC2305_OK) {
        //     printf("Error while ramping to MAX RPM\r\n");
        //     set_faultBit(FAN_CHIP_ERROR);
        // }; 
        // for (uint16_t i = 0; i < 30; i++) {
        //     printf("Fan RPM: %d\r\n", EMC2305_GetFanRPM(&chip, EMC2305_FAN2));
        //     vTaskDelay(pdMS_TO_TICKS(500));
        // }   
        
        printf("successfully ramped :)\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
        
    }
}

int main (void) {

    HAL_Init();
    SystemClock_Config();
    
    xTaskCreateStatic(
        vFanChipTestTask,
        "FanTest",
        TEST_TASK_STACK_SIZE,
        NULL,
        TEST_TASK_PRIORITY,
        xTestStack,
        &xTestTaskBuffer
    );

    vTaskStartScheduler();

    while (true)
    {
        /* code */
    }
    return 0;
    
}


