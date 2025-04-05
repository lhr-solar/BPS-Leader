#ifndef CAN_FILTER_H
#define CAN_FILTER_H
#include "stm32xx_hal.h"

#define CAN_FILTER_MAX_IDS 56

/**
 * @brief Initializes a CAN filter in Mask Mode for 16-bit IDs. 
 *        Mask specifies which bits of ID we do / do not care about.
 *        To accept all IDs, set both ID and Mask to 0.
 * @param filter Pointer to CAN filter
 * @param id 16-bit ID
 * @param mask Masked bits are the bits we care about. e.g. 0xFFFF means we care about all bits.
 */
void CAN_Filter_Mask_Init(CAN_FilterTypeDef *filter, uint8_t id, uint32_t mask);

/**
 * @brief Initializes a CAN filter in Identifier List Mode for 16-bit IDs. 
 *        Pass in an array of 16-bit IDs with at most 56 IDs 
 *        (4 IDs per filter, 14 filters in filterBank = max 56 IDs).
 * @param filter Pointer to CAN filter
 * @param ID_array Array of 16-bit IDs to accept (must have at most 56 IDs)
 * @param numIDs Number of IDs to set
 * @retval 1 if filters initialized successfully; 0 if error while initializing.
 */
uint8_t CAN_Filter_List_Init(CAN_FilterTypeDef *filter, const uint16_t *ID_array, uint8_t numIDs);

#endif // CAN_FILTER_H