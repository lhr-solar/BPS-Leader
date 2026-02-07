#ifndef COMMON_H
#define COMMON_H

#include "stm32xx_hal.h"

#include <stdio.h>
#include <stdbool.h>
#include "pindef.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <timers.h>

typedef enum {
    NORMAL = 0,   /**< Circuit is disconnected */
    EMERGENCY = 1  /**< Circuit is connected */
} fault_state_t;

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


#endif

