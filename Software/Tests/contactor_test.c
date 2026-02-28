#include "common.h"
#include "StatusLEDs.h"
#include "Contactors.h"

// Task configuration
#define TEST_TASK_STACK_SIZE 256
#define TEST_TASK_PRIORITY   ( tskIDLE_PRIORITY + 1 )
#define DELAY_2S             pdMS_TO_TICKS(2000)

// Static task buffers
static StaticTask_t xTestTaskBuffer;
static StackType_t xTestStack[TEST_TASK_STACK_SIZE];

void vContactorTestTask(void *pvParameters) {

    while (1) {
        
        // 1. Cycle through each contactor: Close then Open with 2s delay
        for (uint8_t i = 0; i < NUM_CONTACTORS; i++) {
            // Close Contactor (GPIO_PIN_SET assumes high = closed)
            LED_set(HEARTBEAT_LED, ON);
            contactor_set((contactor_num_t)i, ON, 100, NORMAL);
            vTaskDelay(DELAY_2S);

            // Open Contactor
            LED_set(HEARTBEAT_LED, OFF);
            contactor_set((contactor_num_t)i, OFF, 100, NORMAL);
            vTaskDelay(DELAY_2S);
        }
    

        // 2. Repeatedly toggle the HV+ Contactor
        // Doing this 10 times before restarting the whole sequence
        for (uint8_t j = 0; j < 20; j++) {
            contactor_set(HV_PLUS_CONTACTOR, ON, 100, NORMAL);
            vTaskDelay(DELAY_2S);
            
            contactor_set(HV_PLUS_CONTACTOR, OFF, 100, NORMAL);
            vTaskDelay(DELAY_2S);
        }   
    }
}

/**
 * @brief Task that cycles through all contactors and then toggles the HV+
 */
int main() {
    // Initialize the contactor hardware and software abstractions

    HAL_Init();

    SystemClock_Config();

    LEDs_init();

    contactor_init();

    xTaskCreateStatic(
        vContactorTestTask,
        "ContactorTest",
        TEST_TASK_STACK_SIZE,
        NULL,
        TEST_TASK_PRIORITY,
        xTestStack,
        &xTestTaskBuffer
    );

    vTaskStartScheduler();

    return 0;
}
