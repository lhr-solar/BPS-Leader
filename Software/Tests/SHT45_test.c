// this test will attempt to communicate with the SHT4x, and print the current temperature
// if blinky stops it broke.


#include "common.h"
#include "I2C_Driver.h"
#include "StatusLEDs.h"
#include "SHT45.h"
#include "DebugPrintf.h"

// Task configuration
#define BLINKY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINKY_TASK_PRIORITY   ( tskIDLE_PRIORITY + 1 )

#define SHT4x_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define SHT4x_TASK_PRIORITY   ( tskIDLE_PRIORITY + 2 )

#define DELAY_1S             pdMS_TO_TICKS(1000)

// Static task buffers
static StaticTask_t xBlinkyTaskBuffer;
static StackType_t xBlinkyStack[BLINKY_TASK_STACK_SIZE];

// Static task buffers
static StaticTask_t xSHT4xTaskBuffer;
static StackType_t xSHT4xStack[BLINKY_TASK_STACK_SIZE];

extern I2C_HandleTypeDef hi2c4;

// blink
void vBlinkyTask(void *pvParameters) {

    bool toggle = true;
    while (true) {
        setHeartbeat(toggle);
        toggle = !toggle;
        vTaskDelay(DELAY_1S);
    }
}

void vSHT4xTask(void *pvParameters) {

    uint32_t tmpHmd_buffer[2];
    while (true) {

        if (SHT45_get(tmpHmd_buffer) != SHT45_OK) {
            Error_Handler();
        };

        vTaskDelay(pdMS_TO_TICKS(500));
        printf("TEMP = %lu   |   HUMIDITY = %lu\n\r", tmpHmd_buffer[TEMP], tmpHmd_buffer[HUMIDITY]);
    }
}


int main() {

    HAL_Init();

    SystemClock_Config();

    SHT45_init();

    LEDs_init();

    debugPrintf_init();

    printf("initialized");

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
        SHT4x_TASK_STACK_SIZE,
        NULL,
        SHT4x_TASK_PRIORITY,
        xSHT4xStack,
        &xSHT4xTaskBuffer
    );
    

    vTaskStartScheduler();
    
    return 0;

}



