#include "common.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"
#include "task.h"

// Task configuration
#define BLINKY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINKY_TASK_PRIORITY   ( tskIDLE_PRIORITY + 1 )
#define DELAY_1S             pdMS_TO_TICKS(1000)

// Static task buffers
static StaticTask_t xBlinkyTaskBuffer;
static StackType_t xBlinkyStack[BLINKY_TASK_STACK_SIZE];

// blink
void vBlinkyTask(void *pvParameters) {
    bool toggle = true;
    while (true) {
        setHeartbeat(toggle);
        toggle ? printf("ON\r\n") : printf("OFF\r\n");
        toggle = !toggle;
        vTaskDelay(DELAY_1S);
    }
}

int main() {

    HAL_Init();

    SystemClock_Config();

    LEDs_init();

    debugPrintf_init();

    xTaskCreateStatic(
        vBlinkyTask,
        "Blinky",
        BLINKY_TASK_STACK_SIZE,
        NULL,
        BLINKY_TASK_PRIORITY,
        xBlinkyStack,
        &xBlinkyTaskBuffer
    );

    vTaskStartScheduler();

    return 0;

}