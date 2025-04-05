#include "CAN_Filter.h"

// To shift standard ID into correct position; see STM32F4 reference manual
#define SHIFT_AMOUNT 5

void CAN_Filter_Mask_Init(CAN_FilterTypeDef *filter, uint8_t id, uint32_t mask) {
  // ID, Mask mode: only accept IDs specified by intersection of ID and masks
  filter->FilterMode = CAN_FILTERMODE_IDMASK;

  // For 16-bit config, can use either high or low
  filter->FilterIdHigh =  id << SHIFT_AMOUNT;
  filter->FilterIdLow =   id << SHIFT_AMOUNT;
  filter->FilterMaskIdHigh =  mask << SHIFT_AMOUNT;
  filter->FilterMaskIdLow =   mask << SHIFT_AMOUNT;
  
  filter->FilterFIFOAssignment = CAN_RX_FIFO0;
  filter->FilterScale = CAN_FILTERSCALE_16BIT;
  filter->FilterActivation = ENABLE;
}


uint8_t CAN_Filter_List_Init(CAN_FilterTypeDef *filter, const uint16_t *ID_array, uint8_t numIDs) {
  // Can only accept up to 56 IDs in list
  if (numIDs > CAN_FILTER_MAX_IDS) return 0;

  // If NULL array or no IDs, accept all IDs
  if (ID_array == NULL || numIDs == 0) {
    // Actually set to Mask mode
    filter->FilterMode = CAN_FILTERMODE_IDMASK;

    // Set IDs to 0 to accept all IDs
    filter->FilterIdHigh = 0;
    filter->FilterIdLow = 0;
    filter->FilterMaskIdHigh = 0;
    filter->FilterMaskIdLow = 0;

    filter->FilterFIFOAssignment = CAN_RX_FIFO0;
    filter->FilterScale = CAN_FILTERSCALE_16BIT;
    filter->FilterActivation = ENABLE;
    return 1;
  }

  // Otherwise, set up ID filters
  filter->FilterMode = CAN_FILTERMODE_IDLIST;

  // 14 filters in bank, 4 IDs per filter
  // Loop through IDs and init each necessary filter in filterBank 
  uint8_t j = 0;
  for (uint8_t i = 0; i < numIDs; ++i) {
    switch (i % 4) {
      case 0:
      // Access correct filter bank at index
        filter->FilterBank = j;
        j += 1;
        // First ID
        filter->FilterIdHigh = ID_array[i] << SHIFT_AMOUNT;
        break;
      case 1:
        // Second ID
        filter->FilterIdLow = ID_array[i] << SHIFT_AMOUNT;
        break;
      case 2:
        // Third ID
        filter->FilterMaskIdHigh = ID_array[i] << SHIFT_AMOUNT;
        break;
      case 3:
        // Fourth ID
        filter->FilterMaskIdLow = ID_array[i] << SHIFT_AMOUNT;
        break;

      default:
        break;
    }
  }

  // Everything is currently passed to FIFO 0
  filter->FilterFIFOAssignment = CAN_RX_FIFO0;

  // 16-bit scale means we can store 4 bits per filter (2 in ID, 2 in Mask)
  filter->FilterScale = CAN_FILTERSCALE_16BIT;

  filter->FilterActivation = ENABLE;
  return 1;
}
