#include "I2C_Driver.h"
#include "EMC2305.h"
#include "SHT45.h"

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

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    Error_Handler();
}



