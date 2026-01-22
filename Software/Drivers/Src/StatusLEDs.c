#include "StatusLEDs.h"    

static uint16_t LEDbitmap = 0;
uint8_t modFaultBitmap = 0; 
bool debug_LED = 0;

// Shift-Reg values loaded back to front (bitmap first bit is heartbeat, last is AmpIn)
void updateStatusLEDs() {

    // load bits non-modfault LEDS into shift regs (heartbeat is first in, WatchdogErr in last)
    for (uint8_t bit_num = 0; bit_num < (LED_NUM - MOD_FAULT_NUM - 1); bit_num++) {

        loadBit((bool)(LEDbitmap & (1 << bit_num))); 
    }

    // loads mod fault into shift regs, (MSB in first, LSB in last)
    for (int8_t modFault = 4; modFault >= 0; modFault++) {

        loadBit((bool)(LEDbitmap & (1 << modFault)));
    }

    pushLEDS();
}  

void setLED(Fault_Mapping_t LED, bool state) {
    // clears specified bit
    LEDbitmap &= ~LED;
    // sets bit if state = 1, otherwise stays cleared if state = 0
    LEDbitmap |= (LED & state); 
}   

void loadBit(bool bit) {
    // sets input to specified bit, then pulses the clock
    HAL_GPIO_WritePin(LED_SER_PORT, LED_SER_PIN_NUM, bit);
    HAL_GPIO_WritePin(LED_SRCLK_PORT, LED_SRCLK_PIN_NUM, 1);
    HAL_GPIO_WritePin(LED_SRCLK_PORT, LED_SRCLK_PIN_NUM, 0);
}

void pushLEDS() {
    // pushes loaded values to output
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_RCLK_PIN_NUM, 1);
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_RCLK_PIN_NUM, 0);
}

void init_StatusLEDs() {

    /*
    uint8_t modFaultBitmap = 0;
    bool debug_LED = 0;
    */
    
}