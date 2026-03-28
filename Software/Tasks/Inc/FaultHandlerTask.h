#pragma once

#include "common.h"
#include "faultHandler.h"
#include "StatusLEDs.h"
#include "Contactors.h"

/** * @brief Maximum number of characters allowed in a fault description string. 
 */
#define MAX_FAULT_STRING_CHARS 20

/**
 * @brief Structure to hold a fault description string and its active size.
 */
typedef struct {
    const char fault_string[MAX_FAULT_STRING_CHARS]; /**< Text description of the fault */
    uint8_t faultStringSize;                         /**< Actual active length of the string */
} fault_string_t;


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
 * @note This loop typically halts standard system operation, opens contactors, 
 * and continuously broadcasts the fault state (e.g., via CAN or LEDs).
 */
void Fault_Loop(void);

/** * @brief Maps the current internal fault state to the physical diagnostic LEDs. 
 */
void Set_Fault_LED(void);