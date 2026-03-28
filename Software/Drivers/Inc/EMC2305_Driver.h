#pragma once

#include "common.h"
#include "EMC2305.h"

extern I2C_HandleTypeDef hi2c3;

/** * @brief Minimum allowed fan speed in RPM. 
 */
#define FAN_MIN_RPM 3200

/** * @brief Maximum allowed fan speed in RPM. 
 */
#define FAN_MAX_RPM 8000

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