#include "StatusLEDs.h"    

// Shift-Reg values loaded back to front (bitmap first bit is heartbeat, last is AmpIn)
void updateStatusLEDs(uint16_t LEDbitmap) {
    uint16_t bit_mask = 1 << 0;

    // load bits into shift regs from the last to the first
    for (uint8_t bit_num = 0; bit_num < LED_NUM; bit_num++) {
        loadBit(LEDbitmap & bit_mask);
        bit_mask <<= 1;
    }

    // pushes loaded values to output
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_RCLK_PIN_NUM, 1);
}

static void loadBit(bool bit) {
    // sets input to specified bit, then pulses the clock
    HAL_GPIO_WritePin(LED_SER_PORT, LED_SER_PIN_NUM, bit);
    HAL_GPIO_WritePin(LED_SRCLK_PORT, LED_SRCLK_PIN_NUM, 1);
    HAL_GPIO_WritePin(LED_SRCLK_PORT, LED_SRCLK_PIN_NUM, 0);
}

void init_StatusLEDs() {

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // innit channel A GPIO pins
    GPIO_InitStruct.Pin = LED_SER_PIN_NUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OP_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // init channel B GPIO pins
    GPIO_InitStruct.Pin = LED_SRCLK_PIN_NUM | LED_RCLK_PIN_NUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OP_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
}