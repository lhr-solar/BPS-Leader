#pragma once

#include "common.h"
#include "CAN_FD.h"

/** * @brief External handle for the FDCAN1 peripheral (routed to BPS).
 */
extern FDCAN_HandleTypeDef* hfdcan1;

/** * @brief External handle for the FDCAN3 peripheral (routed to CAR).
 */
extern FDCAN_HandleTypeDef* hfdcan3;

/** * @brief Initializes and starts both the BPS and CAR CAN nodes simultaneously. 
 */
can_status_t CAN_Init(void);

/** * @brief Initializes a standard Transmit (TX) Header for FDCAN communication.
 * @note  USE MACROS FOR DATA LENGTH! (e.g., FDCAN_DLC_BYTES_8). Do not pass 
 * raw integer values for the length.
 * * @param tx_header  Pointer to the FDCAN_TxHeaderTypeDef structure to configure.
 * @param ID         The CAN message identifier (Standard or Extended).
 * @param dataLength The Data Length Code (DLC) macro representing the payload size.
 */
void FDCAN_Init_TXHeader(FDCAN_TxHeaderTypeDef* tx_header, uint32_t ID, uint32_t dataLength);