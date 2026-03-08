#pragma once

#include "common.h"
#include "CAN_FD.h"

extern FDCAN_HandleTypeDef* hfdcan1;
extern FDCAN_HandleTypeDef* hfdcan3;

void BPS_CAN_Init();  // init and start BPS CAN
void CAR_CAN_Init();  // init and start CAR CAN
void ALL_CAN_Init();  // init and start both CANs