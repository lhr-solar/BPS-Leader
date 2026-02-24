#include "common.h"

#define FAN_I2C_CHANNEL I2C3
#define TMPHMD_I2C_CHANNEL I2C4

#define BASE_HAL_INTERRUPT_PRIORITY 5

/** * @brief I2C Transmit Complete ISR.
 * Routes I2C transmit interrupt to Fan or Temp humidity sensor based on channel. 
 */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);

/** * @brief I2C Receive Complete ISR.
 * Routes I2C receive interrupt to Fan or Temp humidity sensor based on channel. 
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

/** @brief Configures I2C interrupts and NVIC priorities. */
void i2cInterrupt_init(void);

/** @brief Configures I2C hardware peripherals (baud rate, pins, etc.). */
void i2c_init(void);