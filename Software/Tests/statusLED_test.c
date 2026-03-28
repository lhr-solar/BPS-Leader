// TEST DESCIRPTION:
/* This test should blink all fault LED's 3 times, cycle through count up from 0 to 31 in binary on the Mod Fault LEDs,
then blink DEBUG 3 times. After, it should do a wave down down the board. Ideally. */

#include "common.h"
#include "StatusLEDs.h"
#include "BPS_Tasks.h"

const uint8_t repetitions = 3;
const uint16_t delayMS = pdMS_TO_TICKS(750);
const uint8_t shortDelayMS = pdMS_TO_TICKS(141);

StaticTask_t task_buffer;
StackType_t task_stack[512];

static void PrettyLEDWave() {

    LED_set(0, LED_ON);
    vTaskDelay(shortDelayMS);

    for (led_mapping_t led_num = 1; led_num < FAULT_LED_NUM; led_num++) {
        LED_set(led_num, LED_ON);
        LED_set(led_num - 1, LED_OFF);
        vTaskDelay(shortDelayMS);
    }
 
    LED_set(FAULT_LED_NUM - 1, LED_OFF);

    for (int8_t led_num = MOD_FAULT_BITS - 1; led_num >= 0; led_num--) {
        LEDsModFaultBitmap_set(1 << led_num);
        vTaskDelay(shortDelayMS);
    }

    LEDs_clear();
    
    LED_set(DEBUG_LED, LED_ON);
    vTaskDelay(shortDelayMS);
    LED_set(DEBUG_LED, LED_OFF);
}



static void task(void *pvParameters) {

    LEDs_init();

    while (true) {

    vTaskDelay(delayMS);

    // blink fault LEDs 3 times
    for (uint8_t i = 0; i < repetitions; i++) {

        // turn on all fault-mapping LEDs
        for (led_mapping_t led_num = 0; led_num < FAULT_LED_NUM; led_num++) {
            LED_set(led_num, LED_ON);
        }
        vTaskDelay(delayMS);

        // turn off all fault-mapping LEDs (manually, to ensure function works properly)
        for (led_mapping_t led_num = 0; led_num < FAULT_LED_NUM; led_num++) {
            LED_set(led_num, LED_OFF);
        }
        vTaskDelay(delayMS);
    }


    // iterate through every mod-fault code
    for (uint8_t i = 0; i < (1 << MOD_FAULT_BITS); i++) {
        LEDsModFaultBitmap_set(i);
        vTaskDelay(shortDelayMS);
    }

    vTaskDelay(delayMS);
    LEDs_clear();

    // flash DEBUG LED 3 times
    for (uint8_t i = 0; i < repetitions; i++) {

        LED_set(DEBUG_LED, LED_ON);
        vTaskDelay(delayMS/2);

        LED_set(DEBUG_LED, LED_OFF);
        vTaskDelay(delayMS/2);

    }

    LEDs_clear();
    vTaskDelay(delayMS);

    // LED Wave (pretty)  
    PrettyLEDWave();

    }
}

int main() {

    HAL_Init();

    SystemClock_Config();

    xTaskCreateStatic(
                task,
                "task",
                TEST_TASK_STACK_SIZE,
                NULL,
                TEST_TASK_PRIORITY,
                task_stack,
                &task_buffer);

    
    vTaskStartScheduler();
    while(1){

    }
}

