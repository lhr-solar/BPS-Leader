#include "StatusLEDs.h"
#include "common.h"

const uint8_t repetitions = 3;

const uint16_t delayMS = 750;
const uint8_t shortDelayMS = 141;

static void PrettyLEDWave() {

    LEDs_set(0, ON);
    HAL_Delay(shortDelayMS);

    for (Fault_Mapping_t led_num = 1; led_num < FAULT_LED_NUM; led_num++) {
        LEDs_set(led_num, ON);
        LEDs_set(led_num - 1, OFF);
        HAL_Delay(shortDelayMS);
    }

    LEDs_set(FAULT_LED_NUM - 1, OFF);

    for (int8_t led_num = MOD_FAULT_BITS - 1; led_num >= 0; led_num--) {
        LEDsModFaultBitmap_set(1 << led_num);
        HAL_Delay(shortDelayMS);
    }

    LEDs_clear();
    
    LEDs_set(DEBUG_LED, ON);
    HAL_Delay(shortDelayMS);
    LEDs_set(DEBUG_LED, OFF);
}

int main() {

    HAL_Init();

    SystemClock_Config();

    LEDs_init();

    HAL_Delay(delayMS);

    // blink fault LEDs 3 times
    for (uint8_t i = 0; i < repetitions; i++) {

        // turn on all fault-mapping LEDs
        for (Fault_Mapping_t led_num = 0; led_num < FAULT_LED_NUM; led_num++) {
            LEDs_set(led_num, ON);
        }
        HAL_Delay(delayMS);

        // turn off all fault-mapping LEDs (manually, to ensure function works properly)
        for (Fault_Mapping_t led_num = 0; led_num < FAULT_LED_NUM; led_num++) {
            LEDs_set(led_num, OFF);
        }
        HAL_Delay(delayMS);
    }


    // iterate through every mod-fault code
    for (uint8_t i = 0; i < (1 << MOD_FAULT_BITS); i++) {
        LEDsModFaultBitmap_set(i);
        HAL_Delay(shortDelayMS);
    }

    HAL_Delay(delayMS);
    LEDs_clear();

    // flash DEBUG LED 3 times
    for (uint8_t i = 0; i < repetitions; i++) {

        LEDs_set(DEBUG_LED, ON);
        HAL_Delay(delayMS/2);

        LEDs_set(DEBUG_LED, OFF);
        HAL_Delay(delayMS/2);

    }

    HAL_Delay(delayMS);
    LEDs_clear();
    HAL_Delay(delayMS);

    // LED Wave (pretty)
    PrettyLEDWave();

    while (true) {}

    return 0;
}

