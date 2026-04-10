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

extern bool fault_task_initialized;

typedef enum
{
    // BPS main saefty loop faults
    BPS_FAULT,                  // If any major fault is detected; indicates we're in an emergency state
    CELL_OVERVOLTAGE_FAULT,  // Cell voltage is greater than OVERVOLTAGE_THRESHOLD  (from volt-temp)
    CELL_UNDERVOLTAGE_FAULT, // Cell voltage is less than UNDERVOLTAGE_THRESHOLD    (from volt-temp)
    BQ_CHIP_FAULT,                // BQ Chip faulted or not responding    (from volt-temp)
    CELL_OVERTEMP_FAULT,     // Cell temperature is too high (from volt-temp)
    PACK_OVERCURRENT_FAULT,  // Battery Pack Current is too high    (from amperes)
    CONTACTOR_HV_PLUS_FAULT,    // Contactor does not match expected state (Very bad 😡)
    CONTACTOR_HV_MINUS_FAULT,   // Contactor does not match expected state (Very bad 😡)
    CONTACTOR_ARRAY_FAULT,      // Contactor does not match expected state (Very bad 😡)
    CONTACTOR_ARRAY_PRE_FAULT,  // Contactor does not match expected state (Very bad 😡)

    CONTACTOR_CALLBACK_FAULT, // Callback failed (RIP Driver)
    BOARD_OVERTEMP_FAULT,     // Board ambient temperature is too high    (from SHT45)
    BPS_ESTOP1_FAULT,         // ESTOP1 pressed, or you forgot the jumpers
    BPS_ESTOP2_FAULT,         // ESTOP2 pressed, or you forgot the jumpers
    BPS_ESTOP3_FAULT,         // ESTOP3 pressed, or you forgot the jumpers

    // Precharge faults
    ARRAY_GREATER_THAN_BATTERY_FAULT, // Array voltage is greater than battery voltage (from precharge ADC signal)
    PRECHARGE_TIMEOUT_FAULT,          // Precharge sequence took too long
    PRECHARGE_HYSTERESIS_FAULT,       // Precharge Array voltage fell under hysteresis threshold while precharging
    // PACK_OVERVOLTAGE_FAULT,           // Pack voltage is too high (Precharge ADC reading)
    // PACK_UNDERVOLTAGE_FAULT,          // Pack voltage is too low (Precharge ADC reading)

    // Software Errors
    RTOS_WATCHDOG_ERROR, // Watchdog did not get pet in time, code is likely blocking somewhere
    BPS_CAN_ERROR,       // BPS CAN failed a send or receive after configured retries
    CAR_CAN_ERROR,       // CAR CAN failed a send or receive after configured retries
    I2C_ERROR,           // I2C failed communication after configured retries (with fan chip or SHT45)
    ADC_ERROR,           // Error with ADC
    FAN_CHIP_ERROR,      // Fan chip not responding or not responding properly

    NUM_FAULTS,
} fault_bit_t;

/* Convert enum to bitmask */
#define FAULT_BIT(fault) (1UL << (fault))

_Static_assert(NUM_FAULTS <= MAX_FAULT_BITS, "Too many fault bits for EventGroup");

/**
 * @brief Initializes fault bitmap
 *
 * @param none
 * @return 0 on failure, 1 on success. If already initialized returns 1.
 */
uint8_t faultHandler_init(void);

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
void set_faultBitFromISR(fault_bit_t bit, BaseType_t* xHigherPriorityTaskWoken);
