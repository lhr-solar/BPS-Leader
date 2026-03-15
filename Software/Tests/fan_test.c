#include "common.h"
#include "StatusLEDs.h"
#include "EMC2305_Driver.h"
#include "DebugPrintf.h"
#include "BPS_Tasks.h"

extern EMC2305_HandleTypeDef chip;


// Static task buffers
static StaticTask_t xTestTaskBuffer;
static StackType_t xTestStack[TEST_TASK_STACK_SIZE];


void vFanChipTestTask(void *pvParameters) {

    debugPrintf_init();

    LEDs_init();

    printf("printf init successfull\n\r");
    vTaskDelay(250);

    EMC2305_I2C_init();

    printf("i2c init successfull\n\r");

    EMC2305_Driver_init();

    printf("EMC2305 init successfull\n\r");
    printf("Fan ramping to min RPM\n\r");

    while (true) {

        if (EMC2305_SetFanRPM(&chip, EMC2305_FAN1, FAN_MIN_RPM) != EMC2305_OK) {
            printf("Error while ramping to min RPM\n\r");
            Error_Handler();
        }; 
        for (uint16_t i = 0; i < 30; i++) {
            printf("Fan RPM: %d\n\r", EMC2305_GetFanRPM(&chip, EMC2305_FAN1));
            vTaskDelay(pdMS_TO_TICKS(500));
        }   

        vTaskDelay(pdMS_TO_TICKS(2000));
        printf("\n\n\rNow Ramping to: FAN MAX RPM\n\n\r");

        if (EMC2305_SetFanRPM(&chip, EMC2305_FAN1, FAN_MAX_RPM) != EMC2305_OK) {
            printf("Error while ramping to MAX RPM\n\r");
            Error_Handler();
        }; 
        for (uint16_t i = 0; i < 30; i++) {
            printf("Fan RPM: %d\n\r", EMC2305_GetFanRPM(&chip, EMC2305_FAN1));
            vTaskDelay(pdMS_TO_TICKS(500));
        }   
        
        }
        printf("successfully ramped :)\n\r");
        
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


