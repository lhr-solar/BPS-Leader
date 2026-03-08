#pragma once

#include "common.h"
#include <event_groups.h>
#include <stdint.h>

#define ALL_FAULT_BITS ((1UL << NUM_FAULTS) - 1UL)

// The max number of fault bits is dependent on the configUSE_16_BIT_TICKS defined in FreeRTOS.h
#if (configUSE_16_BIT_TICKS == 0)
#define MAX_FAULT_BITS 24U
#else
#define MAX_FAULT_BITS 8U
#endif

typedef enum
{
    PRECHARGE_INITIAL_STATE,          // Indiciates we are in the inital state when set
    PRECHARGE_PRECHARGING_STATE,      // Indicates we are in the precharging state when set
    PRECHARGE_RUN_STATE,              // Indicates we are in the run state when set
    ARRAY_GREATER_THAN_BATTERY_FAULT, // Array voltage is greater than battery voltage
    BATTERY_OVERVOLTAGE_FAULT,        // Battery voltage is greater than OVERVOLTAGE_THRESHOLD_MV
    BATTERY_UNDERVOLTAGE_FAULT,       // Battery voltage is less than UNDERVOLTAGE_THRESHOLD_MV
    ARRAY_SENSE_TIMEOUT_FAULT,        // Array contactor didn't close within expected time
    PRECHARGE_SENSE_TIMEOUT_FAULT,    // Precharge contactor didn't close within expected time
    PRECHARGE_TIMEOUT_FAULT,          // Precharge sequence took too long
    CALLBACK_FAULT,                   // Contactor state did not match expected state after being set
    ARRAY_SENSE_MISMATCH_FAULT,       // Array contactor sense pin reading does not match contactor state
    PRECHARGE_SENSE_MISMATCH_FAULT,   // Precharge contactor sense pin reading does not match contactor state
    NUM_FAULTS
} fault_bit_t;

/* Convert enum to bitmask */
#define FAULT_BIT(fault) (1UL << (fault))

/* Mask containing only the actual fault bits (exclude precharge state bits)
    Precharge state enum values are the first entries, so keep bits from
    Array_GREATER_THAN_BATTERY_FAULT upwards. */
#define FAULTS_ONLY_MASK ((EventBits_t)(ALL_FAULT_BITS & ~((1UL << (ARRAY_GREATER_THAN_BATTERY_FAULT)) - 1UL)))

/* Legacy name kept for callers that expect a mask of fault bits */
#define FAULT_BITMASK (FAULTS_ONLY_MASK)

_Static_assert(NUM_FAULTS <= MAX_FAULT_BITS, "Too many fault bits for EventGroup");

/**
 * @brief Initializes fault bitmap
 *
 * @param none
 * @return 0 on failure, 1 on success
 */
uint8_t faultBits_init(void);

/**
 * @brief Set a fault in the fault bitmap
 *
 * @param bit which fault is being set
 * @return none
 */
void set_faultBit(fault_bit_t bit);

/**
 * @brief Wait for a fault to be set
 *
 * @param bit which fault to wait for, pass NUM_FAULTS if waiting for any fault
 * @param xTicksToWait delay when waiting
 * @return the event bit that was set
 */
EventBits_t faultBit_wait(fault_bit_t bit, TickType_t xTicksToWait);

/**
 * @brief Set a fault in the fault bitmap from an ISR
 *
 * @param bit which fault is being set
 * @return none
 */
void set_faultBitFromISR(fault_bit_t bit);