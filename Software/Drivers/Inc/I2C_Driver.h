#include "common.h"

#define FAN_I2C_CHANNEL I2C3
#define TMPHMD_I2C_CHANNEL I2C4

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

void i2cInterrupt_init(void);

void i2c_Init(void);