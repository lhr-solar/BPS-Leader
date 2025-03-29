#include "CAN_Filter.h"

void CAN_Filter_Mask_Init(CAN_FilterTypeDef *filter, uint8_t id, uint8_t filterBank, uint32_t mask) {
    // ID, Mask mode: only accept IDs specified by intersection of ID and masks
    filter->FilterBank = filterBank;
    filter->FilterMode = CAN_FILTERMODE_IDMASK;
  
    filter->FilterScale = CAN_FILTERSCALE_32BIT;
  
    // For 16-bit config, high is MS bytes and low is LS bytes
    filter->FilterIdHigh = 0x0000;
    filter->FilterIdLow = id;
    filter->FilterMaskIdHigh = 0x0000;
    filter->FilterMaskIdLow = mask;
  
    filter->FilterFIFOAssignment = CAN_RX_FIFO0;
    filter->FilterActivation = ENABLE;
    filter->SlaveStartFilterBank = 14;
  }