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
    // BPS main saefty loop faults
    BPS_FAULT,                          // If any major fault is detected; indicates we're in an emergency state
    CELL_OVERVOLTAGE_FAULT,             // Cell voltage is greater than OVERVOLTAGE_THRESHOLD  (from volt-temp)
    CELL_UNDERVOLTAGE_FAULT,            // Cell voltage is less than UNDERVOLTAGE_THRESHOLD    (from volt-temp)
    BQ_CHIP_FAULT,                      // BQ Chip faulted or not responding    (from volt-temp)
    CELL_OVERTEMP_FAULT,                // Cell temperature is too high (from volt-temp)
    PACK_OVERCURRENT_CHARGING_FAULT,    // Battery Pack Discharging Current is too high    (from amperes)
    PACK_OVERCURRENT_DISCHARGING_FAULT, // Battery Pack Discharging Current is too high    (from amperes)
    CONTACTOR_HV_PLUS_FAULT,            // Contactor does not match expected state (Very bad 😡)
    CONTACTOR_HV_MINUS_FAULT,           // Contactor does not match expected state (Very bad 😡)
    CONTACTOR_ARRAY_FAULT,              // Contactor does not match expected state (Very bad 😡)
    CONTACTOR_ARRAY_PRE_FAULT,          // Contactor does not match expected state (Very bad 😡)

    CONTACTOR_CALLBACK_FAULT, // Callback failed (RIP Driver)
    AMPERES_WATCHDOG_FAULT,   // Amperes board not responding
    BPS_ESTOP1_FAULT,         // ESTOP1 pressed, or you forgot the jumpers
    BPS_ESTOP2_FAULT,         // ESTOP2 pressed, or you forgot the jumpers
    BPS_ESTOP3_FAULT,         // ESTOP3 pressed, or you forgot the jumpers

    // Precharge faults
    ARRAY_GREATER_THAN_BATTERY_FAULT, // Array voltage is greater than battery voltage (from precharge ADC signal)
    PRECHARGE_TIMEOUT_FAULT,          // Precharge sequence took too long
    PRECHARGE_HYSTERESIS_FAULT,       // Precharge Array voltage fell under hysteresis threshold while precharging
    PACK_OVERVOLTAGE_FAULT,           // Pack voltage is too high (Precharge ADC reading)
    PACK_UNDERVOLTAGE_FAULT,          // Pack voltage is too low (Precharge ADC reading)

    // Software Errors
    RTOS_WATCHDOG_ERROR, // Watchdog did not get pet in time, code is likely blocking somewhere
    BPS_CAN_ERROR,       // BPS CAN failed a send or receive after configured retries
    CAR_CAN_ERROR,       // CAR CAN failed a send or receive after configured retries
    FAN_CHIP_ERROR,      // Fan chip not responding or not responding properly
    ELCON_FAULT,         // ELCON faulted while chargin

    NUM_FAULTS
} fault_bit_t;

extern const char* const fault_bit_strings[NUM_FAULTS];

/* Convert enum to bitmask */
#define FAULT_BIT(fault) (1UL << (fault))

_Static_assert(NUM_FAULTS <= MAX_FAULT_BITS*2, "Too many fault bits for EventGroup");


extern bool fault_task_initialized;
extern SemaphoreHandle_t faultSemaphore;

extern bool system_has_faulted;
extern volatile uint32_t first_fault_id;

extern EventGroupHandle_t faultBits_1;
extern EventGroupHandle_t faultBits_2; 

/**
 * @brief Set a fault in the fault bitmap
 *
 * @param bit which fault is being set
 * @return none
 */
#define set_faultBit(bit_index) \
    do { \
        if (fault_task_initialized && ((bit_index) < NUM_FAULTS)) { \
            /* ATOMIC LATCH: Only the very first execution succeeds */ \
            taskENTER_CRITICAL(); \
            if (!system_has_faulted) { \
                first_fault_id = (bit_index); /* Save the root cause! */ \
                system_has_faulted = true; \
            } \
            taskEXIT_CRITICAL(); \
            \
            /* Continue setting the event group normally to wake the handler */ \
            EventGroupHandle_t target_group = ((bit_index) < MAX_FAULT_BITS) ? faultBits_1 : faultBits_2; \
            xEventGroupSetBits(target_group, FAULT_BIT((bit_index) % MAX_FAULT_BITS)); \
            xSemaphoreGive(faultSemaphore); \
        } \
    } while(0)

/**
 * @brief Set a fault in the fault bitmap from an ISR
 *
 * @param bit which fault is being set
 * @param pxHigherPriorityTaskWoken address of pxHigherPriorityTaskWoken
 * @return none
 */
#define set_faultBitFromISR(bit_index, pxHigherPriorityTaskWoken) \
    do { \
        /* Check initialization AND ensure bit is within valid total range */ \
        if (fault_task_initialized && ((bit_index) < NUM_FAULTS)) { \
            \
            /* ATOMIC LATCH: Capture the very first fault, ignore subsequent ones */ \
            UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR(); \
            if (!system_has_faulted) { \
                first_fault_id = (bit_index); \
                system_has_faulted = true; \
            } \
            taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);\
            /* Set the event group bit to wake the handler task */ \
            xEventGroupSetBitsFromISR( \
                ((bit_index) < MAX_FAULT_BITS) ? faultBits_1 : faultBits_2, \
                FAULT_BIT((bit_index) % MAX_FAULT_BITS), \
                (pxHigherPriorityTaskWoken) \
            ); \
            xSemaphoreGiveFromISR(faultSemaphore, pxHigherPriorityTaskWoken); \
            portYIELD_FROM_ISR(*pxHigherPriorityTaskWoken); \
        } \
    } while(0)

/**
 * @brief Initializes fault bitmap
 *
 * @param none
 * @return 0 on failure, 1 on success. If already initialized returns 1.
 */
uint8_t faultHandler_init(void);

/**
 * @brief Wait for a fault to be set
 *
 * @param bit which fault to wait for, pass NUM_FAULTS if waiting for any fault
 * @param xTicksToWait delay when waiting
 * @return first_fault_id, the bit index of the first fault bit that was set
 */
uint32_t faultBit_wait(fault_bit_t bit, TickType_t xTicksToWait);

