#ifndef COMMON_H
#define COMMON_H

#include "stm32xx_hal.h"

#include <stdio.h>
#include <stdbool.h>
#include "pindef.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <timers.h>

typedef enum error_state_t {
    NORMAL = 0,
    ERROR = 1
} error_state_t;

// Interrupt priority for the HAL starts at 5 (lower is used for OS, lower number = higher priority)
#define BASE_HAL_INTERRUPT_PRIORITY 5

void Fault_Handler(void);
void Error_Handler(void);
void SystemClock_Config(void);

// highest error-code is 63 (7 debug LEDS to display error )
typedef enum firmware_error_code_t {
    UNKNOWN = 0
    // TODO: assign error codes

} firmware_error_code_t;

typedef struct {
    GPIO_TypeDef* port; // e.g., GPIOA
    uint16_t      pin_num;  // e.g., GPIO_PIN_3
} GpioPin_t;


#endif

