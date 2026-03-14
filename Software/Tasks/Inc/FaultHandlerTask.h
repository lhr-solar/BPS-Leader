#pragma once

#include "common.h"
#include "faultHandler.h"
#include "StatusLEDs.h"
#include "Contactors.h"

#define MAX_FAULT_STRING_CHARS 20

typedef struct{
    const char fault_string[MAX_FAULT_STRING_CHARS];
    uint8_t faultStringSize;
}fault_string_t;


/** @brief Initialize the fault handler task. */
void Init_FaultHandlerTask();

/** @brief Run the fault handler task. */
void Task_FaultHandler();

/** @brief Kill the precharge task. */
void Kill_Precharge_Task();

void Fault_Loop();

void Set_Fault_LED();