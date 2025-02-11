#ifndef _CANBUS_H
#define _CANBUS_H

#include "stm32f4xx.h"
#include "CAN.h"

HAL_StatusTypeDef internalCANInit();
HAL_StatusTypeDef carCANInit();

#endif