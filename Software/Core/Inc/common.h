#ifndef COMMON_H
#define COMMON_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <timers.h>

#include "stm32xx_hal.h"

#include <stdio.h>
#include <stdbool.h>

#include "pindef.h"

typedef enum {
    NORMAL = 0,   // We good
    EMERGENCY = 1  // We bad
} fault_state_t;    

void Fault_Handler(void);
void Error_Handler(void);
void SystemClock_Config(void);

// highest error-code is 63 (7 debug LEDS to display error )
typedef enum firmware_error_code_t {
    UNKNOWN = 0
    // TODO: assign error codes

} firmware_error_code_t;


#endif

