// TEST DESCIRPTION:
/* This test should blink all fault LED's 3 times, cycle through count up from 0 to 31 in binary on the Mod Fault LEDs,
then blink DEBUG 3 times. After, it should do a wave down down the board. Ideally. */

#include "StatusLEDs.h"
#include "common.h"

const uint8_t repetitions = 3;
const uint16_t delayMS = 750;
const uint8_t shortDelayMS = 141;

static void PrettyLEDWave() {

    LED_set(0, ON);
    HAL_Delay(shortDelayMS);

    for (Fault_Mapping_t led_num = 1; led_num < FAULT_LED_NUM; led_num++) {
        LED_set(led_num, ON);
        LED_set(led_num - 1, OFF);
        HAL_Delay(shortDelayMS);
    }
 
    LED_set(FAULT_LED_NUM - 1, OFF);

    for (int8_t led_num = MOD_FAULT_BITS - 1; led_num >= 0; led_num--) {
        LEDsModFaultBitmap_set(1 << led_num);
        HAL_Delay(shortDelayMS);
    }

    LEDs_clear();
    
    LED_set(DEBUG_LED, ON);
    HAL_Delay(shortDelayMS);
    LED_set(DEBUG_LED, OFF);
}

int main() {

    HAL_Init();

    SystemClock_Config();

    LEDs_init();

    while (true) {

    HAL_Delay(delayMS);

    // blink fault LEDs 3 times
    for (uint8_t i = 0; i < repetitions; i++) {

        // turn on all fault-mapping LEDs
        for (Fault_Mapping_t led_num = 0; led_num < FAULT_LED_NUM; led_num++) {
            LED_set(led_num, ON);
        }
        HAL_Delay(delayMS);

        // turn off all fault-mapping LEDs (manually, to ensure function works properly)
        for (Fault_Mapping_t led_num = 0; led_num < FAULT_LED_NUM; led_num++) {
            LED_set(led_num, OFF);
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

        LED_set(DEBUG_LED, ON);
        HAL_Delay(delayMS/2);

        LED_set(DEBUG_LED, OFF);
        HAL_Delay(delayMS/2);

    }

    LEDs_clear();
    HAL_Delay(delayMS);

    // LED Wave (pretty)  
    PrettyLEDWave();

    }

    return 0;
}

