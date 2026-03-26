#pragma once

#include "common.h"
#include "CAN_FD.h"

extern FDCAN_HandleTypeDef* hfdcan1;
extern FDCAN_HandleTypeDef* hfdcan3;

void BPS_CAN_Init();  // init and start BPS CAN
void CAR_CAN_Init();  // init and start CAR CAN
void ALL_CAN_Init();  // init and start both CANs

// Initializes a standard TX Header. USE MACROS FOR DATA LENGTH!!! (i.e. FDCAN_DLC_BYTES_8)
void FDCAN_Init_TXHeader(FDCAN_TxHeaderTypeDef* tx_header, uint32_t ID, uint32_t dataLength); 
