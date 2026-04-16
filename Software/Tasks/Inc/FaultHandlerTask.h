#pragma once

#include "common.h"
#include "faultHandler.h"
#include "StatusLEDs.h"
#include "Contactors.h"
#include "BPS_Tasks.h"

/** * @brief Maximum number of characters allowed in a fault description string.
 */
#define MAX_FAULT_STRING_CHARS 20

/** * @brief Initializes RTOS structures required for fault handling.
 */
void Init_FaultHandlerTask(void);

/** * @brief Main RTOS task loop for monitoring and reacting to system faults.
 * @note This should be spawned by init task and run continuously.
 */
void Task_FaultHandler(void *argument);

/** * @brief Terminates the precharge task.
 * @note Typically called during a critical fault condition where precharge must
 * be aborted immediately to protect the HV system.
 */
void Kill_Precharge_Task(void);

/** * @brief Infinite terminal loop entered upon a critical, unrecoverable system failure.
 * @param fault_bit_index takes the bit index in the bitmap to know which fault called fault handler 
 * @note This loop typically halts standard system operation, opens contactors,
 * and continuously broadcasts the fault state (e.g., via CAN or LEDs).
 */
void Fault_Loop(uint32_t fault_bit_index);

/** * @brief Maps the current internal fault state to the physical diagnostic LEDs.
 */
void Set_Fault_LED(void);