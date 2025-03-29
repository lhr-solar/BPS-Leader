#include "CAN_Filter.h"

void CAN_Filter_Mask_Init(CAN_FilterTypeDef *filter, uint8_t id, uint8_t filterBank, uint32_t mask) {    
    filter->FilterBank = filterBank;
    filter->FilterMode = CAN_FILTERMODE_IDMASK;

    filter->FilterScale = CAN_FILTERSCALE_32BIT;
    filter->FilterIdHigh = id;
    filter->FilterIdLow = 0x0000;
    filter->FilterMaskIdHigh = mask;
    filter->FilterMaskIdLow = 0x0000;
    filter->FilterFIFOAssignment = CAN_RX_FIFO0;
    filter->FilterActivation = ENABLE;
    filter->SlaveStartFilterBank = 14;
}