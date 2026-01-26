#include "I2C_Driver.h"
#include "EMC2305.h"
#include "TempHumiditySensor.h"

I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;


// There is no Error/Nack callback interrupt, the semaphore-take will timeout and flag the error

// routes I2C interrupts to TmpHmd sensor or Fan Chip depending on the I2C channel
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {

    if (hi2c->Instance == TMPHMD_I2C_CHANNEL) {
        SHT4x_I2C_MasterTxRxCpltCallback();
    }
    else if (hi2c->Instance == FAN_I2C_CHANNEL) {
        EMC2305_I2C_MasterTxCpltCallback(hi2c);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {

    if (hi2c->Instance == TMPHMD_I2C_CHANNEL) {
        SHT4x_I2C_MasterTxRxCpltCallback();
    }
    else if (hi2c->Instance == FAN_I2C_CHANNEL) {
        EMC2305_I2C_MasterRxCpltCallback(hi2c);
    }
}

// enables I2C interrupts   
void i2cInterrupt_init() {
    HAL_NVIC_SetPriority(I2C3_EV_IRQn, BASE_HAL_INTERRUPT_PRIORITY + 1, 0);
    HAL_NVIC_SetPriority(I2C4_EV_IRQn, BASE_HAL_INTERRUPT_PRIORITY + 1, 0);
    HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
}

// init I2C Channels
void i2c_Init(void)
{

  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00503D58;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }


  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x00303D5B;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
}
