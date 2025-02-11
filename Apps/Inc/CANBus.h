#ifndef _CANBUS_H
#define _CANBUS_H

#include "stm32f4xx.h"
#include "CAN.h"

#ifndef CAN1
#define CAN1
#endif

#ifndef CAN2
#define CAN2
#endif

// Hardcoded CAN1 pins on the STM32F4
#define CAN1PORT GPIOA
#define CAN1RX GPIP_PIN_11
#define CAN1TX GPIP_PIN_12


HAL_StatusTypeDef internalCANInit();
HAL_StatusTypeDef carCANInit();

#endif