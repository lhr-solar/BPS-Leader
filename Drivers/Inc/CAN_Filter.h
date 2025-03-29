#ifndef CAN_FILTER_H
#define CAN_FILTER_H
#include "stm32xx_hal.h"

void CAN_Filter_Mask_Init(CAN_FilterTypeDef *filter, uint8_t id, uint8_t filterBank, uint32_t mask);

#endif // CAN_FILTER_H