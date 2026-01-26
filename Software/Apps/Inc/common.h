#ifndef COMMON_H
#define COMMON_H

#include "stm32xx_hal.h"

// I hate squiggles
/* #include "stm32g4xx.h"
#include "stm32g411xb.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_i2c_ex.h" */

#include <stdio.h>
#include <stdlib.h>
#include "pindef.h"

#define OPEN 0
#define CLOSE 1

#define NORMAL 0
#define EMERGENCY 1

#define NOT_BLOCKING 0 
#define BLOCKING 1

#define ON 1
#define OFF 0

// Interrupt priority for the HAL starts at 5 (lower is used for OS, lower number = higher priority)
#define BASE_HAL_INTERRUPT_PRIORITY 5

void faultHandler(void);
void errorHandler(void);

// highest error-code is 63 (7 debug LEDS to display error )
typedef enum firmware_error_code_t {
    Unknown = 0
    // TODO: assign error codes

} firmware_error_code_t;

typedef struct {
    GPIO_TypeDef* port; // e.g., GPIOA
    uint16_t      pin_num;  // e.g., GPIO_PIN_3
} GpioPin_t;


#endif

