#ifndef COMMON_H
#define COMMON_H

#include "stm32xx_hal.h"
#include <stdio.h>

#define OPEN 0
#define CLOSE 1

#define NORMAL 0
#define EMERGENCY 1

#define NOT_BLOCKING 0 
#define BLOCKING 1

// highest error-code is 63 (7 debug LEDS to display error )
typedef enum firmware_error_code_t {
    Unknown = 0,
    // TODO: assign error codes

} firmware_error_code_t;




#endif

