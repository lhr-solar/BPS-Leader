#include "common.h"
#include "StatusLEDs.h"
#include "EMC2305_Driver.h"
#include "DebugPrintf.h"

extern EMC2305_HandleTypeDef chip;

// Task configuration
#define TEST_TASK_STACK_SIZE 256
#define TEST_TASK_PRIORITY   ( tskIDLE_PRIORITY + 1 )

// change this 
#define FAN_MAX_RAMP 3000

// Static task buffers
static StaticTask_t xTestTaskBuffer;
static StackType_t xTestStack[TEST_TASK_STACK_SIZE];


void vFanChipTestTask(void *pvParameters) {

    EMC2305_Driver_init();
    debugPrintf_init();

    printf("init successfull");

    while (true) {

        printf("Fan ramping to %d\n\r", FAN_MAX_RAMP);
        for (uint16_t i = 0; i < FAN_MAX_RAMP; i++) {

            if (EMC2305_SetFanRPM(&chip, EMC2305_FAN1, i) != EMC2305_OK) {
                printf("Error while ramping\n\r");
                Error_Handler();
            };
        }
        printf("successfully ramped :)");
        
    }
}




int main (void) {

    HAL_Init();
    SystemClock_Config();

    LEDs_init();
    EMC2305_I2C_init();

    xTaskCreateStatic(
        vFanChipTestTask,
        "FanTest",
        TEST_TASK_STACK_SIZE,
        NULL,
        TEST_TASK_PRIORITY,
        xTestStack,
        &xTestTaskBuffer
    );


}


