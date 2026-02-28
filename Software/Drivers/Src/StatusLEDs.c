#include "common.h"
#include "StatusLEDs.h"    

uint16_t LEDbitmap;
uint8_t modFaultBitmap;

// FreeRTOS Static Mutex Variables
static StaticSemaphore_t xLEDMutexBuffer;
static SemaphoreHandle_t xLEDMutex = NULL;

// sets input to specified bit, then pulses the clock
static void loadBit(bool bit) {
    HAL_GPIO_WritePin(LED_SER_PORT, LED_SER_PIN, bit);
    HAL_GPIO_WritePin(LED_SRCLK_PORT, LED_SRCLK_PIN, ON);
    HAL_GPIO_WritePin(LED_SRCLK_PORT, LED_SRCLK_PIN, OFF);
    HAL_GPIO_WritePin(LED_SER_PORT, LED_SER_PIN, ON);
}

// pushes loaded values to output
static void pushLEDS() {
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_RCLK_PIN, ON);
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_RCLK_PIN, OFF);
}

void setHeartbeat(bool state) {
    HAL_GPIO_WritePin(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, state);
}

// Shift-Reg values loaded back to front (bitmap first bit is heartbeat, last is AmpIn)
static void updateStatusLEDs() {

    // load bits non-modfault LEDS into shift regs (heartbeat is first in, WatchdogErr in last)
    for (uint8_t bit_num = 0; bit_num < FAULT_LED_NUM; bit_num++) {
        loadBit((LEDbitmap & (1 << bit_num)) != 0); 
    }

    // loads mod fault into shift regs, (MSB in first, LSB in last)
    for (int8_t modFault = 4; modFault >= 0; modFault++) {
        loadBit((modFaultBitmap & (1 << modFault)) != 0);
    }

    loadBit((LEDbitmap & (1 << DEBUG_LED)) != 0);

    pushLEDS();
}  

void LEDsModFaultBitmap_set(uint8_t bitmap) {

    // make sure bitmap is in range
    if (bitmap >= (1 << MOD_FAULT_BITS)) {
        Error_Handler();
    }

    if (xLEDMutex != NULL && xSemaphoreTake(xLEDMutex, portMAX_DELAY) == pdTRUE) {
        modFaultBitmap = bitmap;
        updateStatusLEDs();
        xSemaphoreGive(xLEDMutex);
    }

}

void LED_set(Fault_Mapping_t LED_i, bool state) {

    // make sure LED is in range
    if ((LED_i < 0) || ((LED_i >= FAULT_LED_NUM) && (LED_i != DEBUG_LED))) {
        Error_Handler();
    }

    uint16_t LED = 1 << LED_i;

    if (xLEDMutex != NULL && xSemaphoreTake(xLEDMutex, portMAX_DELAY) == pdTRUE) {
        // clears specified bit
        LEDbitmap &= ~LED;
        // sets bit if state = 1, otherwise stays cleared if state = 0
        LEDbitmap |= (state ? LED : 0); 

        updateStatusLEDs();
        xSemaphoreGive(xLEDMutex);
    }
}   

void LEDs_clear() {
    if (xLEDMutex != NULL && xSemaphoreTake(xLEDMutex, portMAX_DELAY) == pdTRUE) {
        LEDbitmap = 0;
        modFaultBitmap = 0;
        updateStatusLEDs();
        xSemaphoreGive(xLEDMutex);
    }
}

void LEDs_init() {

    if (xLEDMutex == NULL) {
        xLEDMutex = xSemaphoreCreateMutexStatic(&xLEDMutexBuffer);
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LED_RCLK_PORT, LED_SRCLK_PIN|LED_RCLK_PIN|LED_SER_PIN, GPIO_PIN_RESET);

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = LED_SRCLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_SRCLK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_RCLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_RCLK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_SER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_SER_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = HEARTBEAT_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(HEARTBEAT_LED_PORT, &GPIO_InitStruct);

    LEDs_clear(); 
}