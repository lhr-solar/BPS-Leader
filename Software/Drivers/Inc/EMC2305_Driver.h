#pragma once

#include "common.h"
#include "EMC2305.h"

// FAN CHIP DATA SHEET
// https://ww1.microchip.com/downloads/aemDocuments/documents/MSLD/ProductDocuments/DataSheets/EMC2301-2-3-5-Data-Sheet-DS20006532A.pdf

extern I2C_HandleTypeDef hi2c3;

/** * @brief Minimum allowed fan speed in RPM. 
 */
#define FAN_MIN_RPM 100

/** * @brief Maximum allowed fan speed in RPM. 
 */
#define FAN_MAX_RPM 4950

/** * @brief Global hardware handle for the EMC2305 fan controller chip. 
 */
extern EMC2305_HandleTypeDef chip;


/** * @brief Initializes the I2C hardware peripheral used to communicate with the EMC2305. 
 */
void EMC2305_I2C_init(void);

/** * @brief Initializes the EMC2305 chip registers and configures the connected fans.
 * @note  ONLY CALL FROM TASK. This function relies on RTOS delays or 
 * blocking calls and must not be called from an ISR or before the scheduler starts.
 * @return EMC2305_Status Returns the success or error state of the initialization.
 */
EMC2305_Status EMC2305_Driver_init(void);

/** * @brief Forces all fans to their maximum RPM.
 * @note  used as a failsafe during system fault conditions.
 */
void set_fans_MAX(void);

/**
 * @brief Sets the target RPM (Revolutions Per Minute) for the specified fan.
 * * @details This function configures the EMC2305's closed-loop Fan Speed Control (FSC) 
 * algorithm to automatically drive the fan to the requested RPM. The controller will 
 * adjust the PWM output dynamically based on the tachometer feedback to maintain this target.
 *
 * @param[in] fan The fan channel to configure (e.g., FAN_1, FAN_2, etc. defined in EMC2305_Fan).
 * @param[in] rpm The target fan speed in Revolutions Per Minute.
 * * @return EMC2305_Status Execution status of the command.
 */
EMC2305_Status set_fan_rpm(EMC2305_Fan fan, uint16_t rpm);

/**
 * @brief Sets the direct PWM (Pulse Width Modulation) drive value for the specified fan.
 * * @details This function is used for open-loop control. It disables the closed-loop 
 * RPM algorithm for the specified channel and forces the PWM output to the requested 
 * duty cycle. 
 *
 * @param[in] fan The fan channel to configure (e.g., FAN_1, FAN_2, etc. defined in EMC2305_Fan).
 * @param[in] pwm The target PWM duty cycle value to apply.
 * * @return EMC2305_Status Execution status of the command.
 */
EMC2305_Status set_fan_pwm(EMC2305_Fan fan, uint16_t pwm);
