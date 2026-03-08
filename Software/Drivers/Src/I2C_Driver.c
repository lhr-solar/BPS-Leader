#include "I2C_Driver.h"
#include "EMC2305.h"
#include "SHT45.h"

extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;

// routes I2C interrupts to TmpHmd sensor or Fan Chip depending on the I2C channel
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {

    if (hi2c->Instance == TMPHMD_I2C_CHANNEL) {
        SHT45_I2C_MasterTxRxCpltCallback();
    }
    else if (hi2c->Instance == FAN_I2C_CHANNEL) {
        EMC2305_I2C_MasterTxCpltCallback(hi2c);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {

    if (hi2c->Instance == TMPHMD_I2C_CHANNEL) {
        SHT45_I2C_MasterTxRxCpltCallback();
    }
    else if (hi2c->Instance == FAN_I2C_CHANNEL) {
        EMC2305_I2C_MasterRxCpltCallback(hi2c);
    }
}

/**
  * @brief This function handles I2C3 event interrupt.
  */
void I2C3_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&hi2c3);
}

/**
  * @brief This function handles I2C3 error interrupt.
  */
void I2C3_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&hi2c3);
}

/**
  * @brief This function handles I2C4 event interrupt.
  */
void I2C4_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&hi2c4);
}

/**
  * @brief This function handles I2C4 error interrupt.
  */
void I2C4_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&hi2c4);
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    Error_Handler();
}



