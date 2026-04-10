#include "common.h"
#include "StatusLEDs.h"
#include "Contactors.h"
#include "DebugPrintf.h"
#include "FaultHandlerTask.h"
#include "BPS_Tasks.h"

// Task configuration
#define DELAY_2S pdMS_TO_TICKS(2000)

// Static task buffers
static StaticTask_t xTestTaskBuffer;
static StackType_t xTestStack[TEST_TASK_STACK_SIZE];

void vContactorTestTask(void *pvParameters)
{
    LEDs_init();

    contactor_init();

    debugPrintf_init();

    printf("initialized\r\n");

    HAL_GPIO_WritePin(HV_PLUS_CONTROL_PORT, HV_PLUS_CONTROL_PIN, GPIO_PIN_SET);

    // if (contactor_set(HV_PLUS_CONTACTOR, CONTACTOR_CLOSED, 10, EMERGENCY) != CONTACTOR_OK)
    // {
    //     printf("failed\r\n");
    // }

    while (1)
    {

        vTaskDelay(DELAY_2S);

        // // 1. Cycle through each contactor: Close then Open with 2s delay
        // for (uint8_t i = 0; i < NUM_CONTACTORS ; i++) {
        //     // Close Contactor (GPIO_PIN_SET assumes high = closed)
        //     LED_set(HEARTBEAT_LED, LED_ON);
        //     contactor_set((contactor_num_t)i, CONTACTOR_CLOSED, 100, EMERGENCY);
        //     vTaskDelay(DELAY_2S);

        //     // Open Contactor
        //     LED_set(HEARTBEAT_LED, LED_OFF);
        //     contactor_set((contactor_num_t)i, CONTACTOR_OPEN, 100, EMERGENCY);
        //     vTaskDelay(DELAY_2S);
        // }

        // // 2. Repeatedly toggle the HV+ Contactor
        // // Doing this 10 times before restarting the whole sequence
        // for (uint8_t j = 0; j < 20; j++) {
        //     contactor_set(HV_PLUS_CONTACTOR, CONTACTOR_CLOSED, 100, NORMAL);
        //     vTaskDelay(DELAY_2S);

        //     contactor_set(HV_PLUS_CONTACTOR, CONTACTOR_OPEN, 100, NORMAL);
        //     vTaskDelay(DELAY_2S);
        // }
    }
}

/**
 * @brief Task that cycles through all contactors and then toggles the HV+
 */
int main()
{
    // Initialize the contactor hardware and software abstractions

    HAL_Init();

    SystemClock_Config();

    // xTaskCreateStatic(
    //     Task_FaultHandler,             // Task function
    //     "FaultHandler",                // Name of the task (for debugging)
    //     configMINIMAL_STACK_SIZE,   // Stack size in words
    //     NULL,                       // Task input parameter
    //     tskIDLE_PRIORITY + 3,       // Task priority
    //     FaultHandler_Task_Stack,       // Task handle
    //     &FaultHandler_Task_Buffer      // Static task buffer (optional)
    // );

    xTaskCreateStatic(
        vContactorTestTask,
        "ContactorTest",
        TEST_TASK_STACK_SIZE,
        NULL,
        TEST_TASK_PRIORITY,
        xTestStack,
        &xTestTaskBuffer);

    vTaskStartScheduler();

    return 0;
}
