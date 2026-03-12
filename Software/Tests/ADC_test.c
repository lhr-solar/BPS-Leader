#include "common.h"
#include "ADC_Driver.h"
#include "DebugPrintf.h"
#include "StatusLEDs.h"

StaticTask_t task_buffer;
StackType_t task_stack[512];

#define TEST_TIMEOUT 20 

void vTestTaskToskTisk(void* pvParameters) {
    
    ADC_Sense_Result ADC_result = {0}; 

    LEDs_init();
    debugPrintf_init();
    printf("Printf initialized!\n\r");
    if (ADC_Sense_Init() != ADC_SENSE_OK) printf("ADC failed initialization :( \n\r");
    printf("ADC initialized!\n\r");

    while (true) {

        setHeartbeat(ON);
        if (Read_ADC(TEST_TIMEOUT, &ADC_result) != ADC_SENSE_OK) printf("ADC failed reading :( \n\r");
        printf("Array voltage: %lu       |       Battery voltage: %lu\n\r", ADC_result.Array_Voltage/1000, ADC_result.Battery_Voltage/1000);
        vTaskDelay(pdMS_TO_TICKS(500));
        setHeartbeat(OFF);
        vTaskDelay(pdMS_TO_TICKS(500));

    }
}

int main() {

    HAL_Init();

    SystemClock_Config();

    xTaskCreateStatic(
                vTestTaskToskTisk,
                "task",
                512,
                NULL,
                tskIDLE_PRIORITY + 2,
                task_stack,
                &task_buffer);

    
    vTaskStartScheduler();
    while(1){

    }
    return 0;
}