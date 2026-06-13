#include "common.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"
#include "Contactors.h"

// Task configuration
#define BLINKY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINKY_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define DELAY_1S pdMS_TO_TICKS(1000)

// Static task buffers
static StaticTask_t xBlinkyTaskBuffer;
static StackType_t xBlinkyStack[BLINKY_TASK_STACK_SIZE];

// blink
void vBlinkyTask(void *pvParameters)
{
    contactor_init();
    while (true)
    {
        vTaskDelay(DELAY_1S / 2);

        switch (contactor_estop_checker())
        {
        case ESTOP_OK:
            printf("ESTOPs not pressed!\r\n");
            break;
        case ESTOP1_FAULT:
            printf("ESTOP 1 pressed!\r\n");
            break;
        case ESTOP2_FAULT:
            printf("ESTOP 2 pressed!\r\n");
            break;
        case ESTOP3_FAULT:
            printf("ESTOP 3 pressed!\r\n");
            break;
        default:
            break;
        }
    }
}

int main()
{

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
        &xBlinkyTaskBuffer);

    vTaskStartScheduler();

    return 0;
}