// this test will attempt to communicate with the SHT4x, and print the current temperature
// if blinky stops it broke.


#include "common.h"
#include "I2C_Driver.h"
#include "StatusLEDs.h"
#include "SHT45.h"
#include "DebugPrintf.h"
#include "BPS_Tasks.h"

// Task configuration
#define BLINKY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINKY_TASK_PRIORITY   ( tskIDLE_PRIORITY + 1 )

#define DELAY_1S             pdMS_TO_TICKS(1000)

// Static task buffers
static StaticTask_t xBlinkyTaskBuffer;
static StackType_t xBlinkyStack[BLINKY_TASK_STACK_SIZE];

// Static task buffers
static StaticTask_t xSHT4xTaskBuffer;
static StackType_t xSHT4xStack[TEST_TASK_STACK_SIZE];

extern I2C_HandleTypeDef hi2c4;

// blink
void vBlinkyTask(void *pvParameters) {

    while (true) {
        toggleHeartbeat();
        vTaskDelay(DELAY_1S);
    }
}

void vSHT4xTask(void *pvParameters) {

    debugPrintf_init();

    printf("printf initialized\r\n");

    LEDs_init();

    SHT45_init();

    int32_t tmpHmd_buffer[2];
    while (true) {

        if (SHT45_get(tmpHmd_buffer, DELAY_1S) != SHT45_OK) {
            Error_Handler();
        };

        vTaskDelay(pdMS_TO_TICKS(500));
        printf("TEMP = %ld   |   HUMIDITY = %ld\r\n", tmpHmd_buffer[SHT45_TEMP], tmpHmd_buffer[SHT45_HUMIDITY]);
    }
}


int main() {

    HAL_Init();

    SystemClock_Config();

    xTaskCreateStatic(
        vBlinkyTask,
        "Blinky",
        BLINKY_TASK_STACK_SIZE,
        NULL,
        BLINKY_TASK_PRIORITY,
        xBlinkyStack,
        &xBlinkyTaskBuffer
    );


    xTaskCreateStatic(
        vSHT4xTask,
        "SHT4xTask",
        TEST_TASK_STACK_SIZE,
        NULL,
        TEST_TASK_PRIORITY,
        xSHT4xStack,
        &xSHT4xTaskBuffer
    );
    

    vTaskStartScheduler();
    
    return 0;

}



