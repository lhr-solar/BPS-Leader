#ifndef _LED_H__
#define _LED_H__

#include "stm32xx_hal.h"
#include "pinConfig.h"

void Heartbeat_Init();
void Heartbeat_Clock_Init();
void Heartbeat_Toggle();
void Heartbeat_WritePin(uint8_t new_pin_val);

#endif