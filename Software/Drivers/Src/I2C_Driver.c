#include "common.h"
#include "EMC2305.h"
#include "SHT45.h"
#include "pindef.h"

#define I2C_ERROR_THRESHOLD (5)

extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;

volatile uint16_t error_counter_sht45 = 0;
volatile uint16_t error_counter_emc2305 = 0;

/**
 * @brief  Tx Transfer completed callback.
 * @note   Routes the successful transmission interrupt to the appropriate 
 * sensor/chip callback based on the I2C hardware instance.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 * the configuration information for the specified I2C.
 */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == SHT45_I2C_CHANNEL) {
        SHT45_I2C_MasterTxRxCpltCallback();
        error_counter_sht45 = 0;
    }
    else if (hi2c->Instance == FAN_I2C_CHANNEL) {
        EMC2305_I2C_MasterTxCpltCallback(hi2c);
        error_counter_emc2305 = 0;
    }
}

/**
 * @brief  Rx Transfer completed callback.
 * @note   Routes the successful reception interrupt to the appropriate 
 * sensor/chip callback based on the I2C hardware instance.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 * the configuration information for the specified I2C.
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == SHT45_I2C_CHANNEL) {
        SHT45_I2C_MasterTxRxCpltCallback();
        error_counter_sht45 = 0;
    }
    else if (hi2c->Instance == FAN_I2C_CHANNEL) {
        EMC2305_I2C_MasterRxCpltCallback(hi2c);
        error_counter_emc2305 = 0;
    }
}

/**
 * @brief This function handles the I2C3 event interrupt.
 * @note  Triggered on successful I2C3 events (Tx/Rx).
 */
void I2C3_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&hi2c3);
}

/**
 * @brief This function handles the I2C3 error interrupt.
 * @note  Triggered on I2C3 errors (NACK, bus error, arbitration loss).
 */
void I2C3_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&hi2c3);
}

/**
 * @brief This function handles the I2C4 event interrupt.
 * @note  Triggered on successful I2C4 events (Tx/Rx).
 */
void I2C4_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&hi2c4);
}

/**
 * @brief This function handles the I2C4 error interrupt.
 * @note  Triggered on I2C4 errors (NACK, bus error, arbitration loss).
 */
void I2C4_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&hi2c4);
}

/**
 * @brief  I2C error callback.
 * @note   Triggered by the HAL when an error occurs on any I2C bus.
 * Sets a global system fault bit from the ISR context.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 * the configuration information for the specified I2C.
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (hi2c->Instance == SHT45_I2C_CHANNEL) {
        error_counter_sht45++;
    }
    else if (hi2c->Instance == FAN_I2C_CHANNEL) {
        error_counter_emc2305++;
    }
    if (error_counter_emc2305 > I2C_ERROR_THRESHOLD) {
        set_faultBitFromISR(FAN_CHIP_ERROR, &xHigherPriorityTaskWoken);
    }   
}