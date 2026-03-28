#pragma once

#include "UART.h"
#include "projdefs.h"
#include "stm32xx_hal.h"
#include "printf.h"

/** * @brief Initializes the hardware and software configuration for debug printing.
 * @note This configures the underlying UART peripheral and links it 
 * to the printf stream to enable 
 * console output over a serial connection.
 */
void debugPrintf_init();