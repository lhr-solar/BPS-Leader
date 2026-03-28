#include "common.h"
#include "ADC_Driver.h"
#include "DebugPrintf.h"
#include "StatusLEDs.h"
#include "BPS_Tasks.h"

StaticTask_t task_buffer;
StackType_t task_stack[TEST_TASK_STACK_SIZE];

#define TEST_TIMEOUT 20 

void vTestTaskToskTisk(void* pvParameters) {
    
    ADC_Sense_Result ADC_result = {0}; 

    LEDs_init();
    debugPrintf_init();
    printf("Printf initialized!\r\n");

    if (ADC_Sense_Init() != ADC_SENSE_OK) {
        printf("ADC failed initialization :( \r\n");
        Error_Handler();
    }
    
    printf("ADC initialized!\r\n");

    while (true) {

        toggleHeartbeat();
        if (Read_ADC(TEST_TIMEOUT, &ADC_result) != ADC_SENSE_OK) printf("ADC failed reading :( \r\n");
        printf("Array voltage: %lu       |       Battery voltage: %lu\r\n", ADC_result.Array_Voltage/1000, ADC_result.Battery_Voltage/1000);
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}

int main() {

    HAL_Init();

    SystemClock_Config();

    xTaskCreateStatic(
                vTestTaskToskTisk,
                "task",
                TEST_TASK_STACK_SIZE,
                NULL,
                TEST_TASK_PRIORITY,
                task_stack,
                &task_buffer);

    
    vTaskStartScheduler();
    while(1){

    }
    return 0;
}