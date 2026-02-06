#include "StatusLEDs.h"    

static uint16_t LEDbitmap;
static uint8_t modFaultBitmap;

// sets input to specified bit, then pulses the clock
static void loadBit(bool bit) {
    HAL_GPIO_WritePin(LED_SER_PORT, LED_SER_PIN_NUM, bit);
    HAL_GPIO_WritePin(LED_SRCLK_PORT, LED_SRCLK_PIN_NUM, ON);
    HAL_GPIO_WritePin(LED_SRCLK_PORT, LED_SRCLK_PIN_NUM, OFF);
    HAL_GPIO_WritePin(LED_SER_PORT, LED_SER_PIN_NUM, ON);
}

// pushes loaded values to output
static void pushLEDS() {
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_RCLK_PIN_NUM, ON);
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_RCLK_PIN_NUM, OFF);
}

// Shift-Reg values loaded back to front (bitmap first bit is heartbeat, last is AmpIn)
static void updateStatusLEDs() {

    // load bits non-modfault LEDS into shift regs (heartbeat is first in, WatchdogErr in last)
    for (uint8_t bit_num = 0; bit_num < FAULT_LED_NUM; bit_num++) {
        loadBit((bool)(LEDbitmap & (1 << bit_num))); 
    }

    // loads mod fault into shift regs, (MSB in first, LSB in last)
    for (int8_t modFault = 4; modFault >= 0; modFault++) {
        loadBit((bool)(modFaultBitmap & (1 << modFault)));
    }

    (LEDbitmap & (1 << DEBUG_LED)) ? loadBit(ON) : loadBit(OFF);

    pushLEDS();
}  

void LEDsModFaultBitmap_set(uint8_t bitmap) {

    // make sure bitmap is in range
    if (bitmap >= (1 << MOD_FAULT_BITS)) {
        Error_Handler();
    }
 
    modFaultBitmap = bitmap;

    updateStatusLEDs();
}

void LED_set(Fault_Mapping_t LED, bool state) {

    // make sure LED is in range
    if ((LED < 0) || ((LED > 9) && (LED != 15))) {
        Error_Handler();
    }

    LED = 1 << LED;

    // clears specified bit
    LEDbitmap &= ~LED;
    // sets bit if state = 1, otherwise stays cleared if state = 0
    LEDbitmap |= (state ? LED : 0); 

    updateStatusLEDs();
}   

void LEDs_clear() {
    LEDbitmap = 0;
    modFaultBitmap = 0;
    updateStatusLEDs();
}

void LEDs_init() {

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_SRCLK_PIN_NUM|LED_RCLK_PIN_NUM|LED_SER_PIN_NUM, GPIO_PIN_RESET);

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = LED_SRCLK_PIN_NUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_SRCLK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_RCLK_PIN_NUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_RCLK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_SER_PIN_NUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_SER_PORT, &GPIO_InitStruct);

    LEDs_clear(); 
}