#pragma once

#include "common.h"
#include <event_groups.h>
#include <stdint.h>
#include <assert.h>


#define ALL_FAULT_BITS ((1UL << NUM_FAULTS) - 1UL)

// The max number of fault bits is dependent on the configUSE_16_BIT_TICKS defined in FreeRTOS.h
#if (configUSE_16_BIT_TICKS == 0)
#define MAX_FAULT_BITS 24U
#else
#define MAX_FAULT_BITS 8U
#endif

#define FAULT_BIT_ARR_SIZE (1 + ((NUM_FAULTS - 1) / MAX_FAULT_BITS))

// NOTE: FAULTS USED IN CAN STATUS TASK ((MUST))
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
    PRECHARGE_OUT_OF_BOUNDS_FAULT,    // Precharge Array voltage fell under hysteresis threshold while precharging
    PACK_OVERVOLTAGE_FAULT,           // Pack voltage is too high (Precharge ADC reading)
    PACK_UNDERVOLTAGE_FAULT,          // Pack voltage is too low (Precharge ADC reading)
    FAN_TACHOMETER_FAULT,             // Fan tach values is reading wrong value

    // Software Errors
    RTOS_WATCHDOG_ERROR, // Watchdog did not get pet in time, code is likely blocking somewhere
    BPS_CAN_ERROR,       // BPS CAN failed a send or receive after configured retries
    CAR_CAN_ERROR,       // CAR CAN failed a send or receive after configured retries
    FAN_CHIP_ERROR,      // Fan chip not responding or not responding properly
    ADC_ERROR,           // Error with ADC  
    ELCON_FAULT,         // ELCON faulted while chargin
    REGEN_FAULT,         // Regen braking active when BPS says it's unsafe
    I2C_ERROR,           // I2C erring

    NUM_FAULTS
} fault_bit_t;

extern const char *const fault_bit_strings[];

extern EventGroupHandle_t faultBits[FAULT_BIT_ARR_SIZE];


/* Convert enum to bitmask */
#define FAULT_BIT(fault) (1UL << (fault))

extern bool fault_bits_initialized;
extern SemaphoreHandle_t faultSemaphore;

extern bool system_has_faulted;
extern volatile uint32_t first_fault_id;


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

/**
 * @brief Gets a specific fault bit. if bit_index = NUM_FAULTS check for any fault
 *
 * @param bit_index which fault bit to check. if bit_index = NUM_FAULTS check for any fault
 * @return true if the bit is set, else false
 */
bool is_fault_set(uint32_t bit_index);

/**
 * @brief Prints the fault and sets relevant LEDS
 *
 * @param fault_bit_index index of what fault is triggered
 */
void handle_fault(uint32_t fault_bit_index);

/**
 * @brief Set a fault in the fault bitmap and gives semaphore to wake fault handler task
 *
 * @param bit which fault is being set
 * @return none
 */
static inline void set_faultBit(uint32_t bit_index)
{
    /* Check initialization AND ensure bit is within valid total range */
    if (fault_bits_initialized && (bit_index < NUM_FAULTS))
    {
        /* ATOMIC LATCH: Only the very first execution succeeds */
        taskENTER_CRITICAL();
        if (!system_has_faulted)
        {
            first_fault_id = bit_index; /* Save the root cause! */
            system_has_faulted = true;
        }
        taskEXIT_CRITICAL();

        EventGroupHandle_t target_group = faultBits[bit_index / MAX_FAULT_BITS];

        // determine if bit is already set. If its not, set it
        if ((target_group != NULL) && ((xEventGroupGetBits(target_group) & FAULT_BIT(bit_index % MAX_FAULT_BITS)) == 0)) {
            xEventGroupSetBits(target_group, FAULT_BIT(bit_index % MAX_FAULT_BITS));

            if (faultSemaphore != NULL) xSemaphoreGive(faultSemaphore);
            
            taskYIELD();
        }
    }
}

/**
 * @brief Set a fault in the fault bitmap and gives semaphore to wake fault handler task from ISR
 *
 * @param bit which fault is being set
 * @param pxHigherPriorityTaskWoken address of pxHigherPriorityTaskWoken
 * @return none
 */
static inline void set_faultBitFromISR(uint32_t bit_index, BaseType_t *pxHigherPriorityTaskWoken)
{
    /* Check initialization AND ensure bit is within valid total range */
    if (fault_bits_initialized && (bit_index < NUM_FAULTS))
    {

        /* ATOMIC LATCH: Capture the very first fault, ignore subsequent ones */
        UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        if (!system_has_faulted)
        {
            first_fault_id = bit_index;
            system_has_faulted = true;
        }
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

        EventGroupHandle_t target_group = faultBits[bit_index / MAX_FAULT_BITS];

        /// Set the event group bit if it isnt already set
        if ((target_group != NULL) && ((xEventGroupGetBitsFromISR(target_group) & FAULT_BIT(bit_index % MAX_FAULT_BITS)) == 0)) {
            xEventGroupSetBitsFromISR(target_group, FAULT_BIT(bit_index % MAX_FAULT_BITS), pxHigherPriorityTaskWoken);
            
            if (faultSemaphore != NULL) xSemaphoreGiveFromISR(faultSemaphore, pxHigherPriorityTaskWoken);

            if (pxHigherPriorityTaskWoken != NULL) portYIELD_FROM_ISR(*pxHigherPriorityTaskWoken);
        }
    }
}
