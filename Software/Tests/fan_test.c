#include "common.h"
#include "StatusLEDs.h"
#include "EMC2305_Driver.h"
#include "DebugPrintf.h"
#include "FaultHandlerTask.h"
#include "BPS_Tasks.h"
extern EMC2305_HandleTypeDef chip;


// Static task buffers
static StaticTask_t xTestTaskBuffer;
static StackType_t xTestStack[TEST_TASK_STACK_SIZE];


void vFanChipTestTask(void *pvParameters) {

    debugPrintf_init();

    LEDs_init();

    printf("printf init successfull\r\n");
    vTaskDelay(250);

    EMC2305_I2C_init();

    printf("i2c init successfull\r\n");

    EMC2305_Driver_init();

    printf("EMC2305 init successfull\r\n");
    printf("Fan ramping to min PWM\r\n");

    while (true) {

        if (EMC2305_SetFanPWM(&chip, EMC2305_FAN1, 0) != EMC2305_OK) {
            printf("Error while ramping to min PWM\r\n");
            Error_Handler();
        }; 
        for (uint16_t i = 0; i < 30; i++) {
            printf("Fan PWM: %d\r\n", EMC2305_GetFanPWM(&chip, EMC2305_FAN1));
            vTaskDelay(pdMS_TO_TICKS(500));
        }   

        vTaskDelay(pdMS_TO_TICKS(2000));
        printf("\n\r\nNow Ramping to: FAN MAX PWM\n\r\n");
        
        if (EMC2305_SetFanPWM(&chip, EMC2305_FAN1, FAN_MAX_PWM) != EMC2305_OK) {
            printf("Error while ramping to MAX PWM\r\n");
            Error_Handler();
        }; 
        for (uint16_t i = 0; i < 30; i++) {
            printf("Fan PWM: %d\r\n", EMC2305_GetFanPWM(&chip, EMC2305_FAN1));
            vTaskDelay(pdMS_TO_TICKS(500));
        }   
        
        printf("successfully ramped :)\r\n");
        
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

    xTaskCreateStatic(
        Task_FaultHandler,             // Task function
        "FaultHandler",                // Name of the task (for debugging)
        configMINIMAL_STACK_SIZE,   // Stack size in words
        NULL,                       // Task input parameter
        tskIDLE_PRIORITY + 3,       // Task priority
        FaultHandler_Task_Stack,       // Task handle
        &FaultHandler_Task_Buffer      // Static task buffer (optional)
    );

    vTaskStartScheduler();

    while (true)
    {
        /* code */
    }
    return 0;
    
}


