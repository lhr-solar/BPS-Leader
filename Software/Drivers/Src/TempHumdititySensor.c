#include "TempHumiditySensor.h"



uint8_t pollCMD = 0xFD;
uint8_t rx_bytes[6];
double tmpHmdBuffer[2];

static void sensorPoll() {

  HAL_I2C_Master_Transmit(&hi2c4, tmpHmdAdresss << 1, &pollCMD, 1);
  vTaskDelay(pdMS_TO_TICKS(15));
  HAL_I2C_Master_Receive(&hi2c1, tmpHmdAdresss << 1, rx_bytes, sizeof(rx_bytes));

}


void readTmpHmd() {
  
  sensorPoll();

  uint8_t *tmpHmdBuffer;
  uint16_t t_ticks = rx_bytes[0] * 256 + rx_bytes[1];
  uint16_t rh_ticks = rx_bytes[3] * 256 + rx_bytes[4];
  tmpHmdBuffer[0] = -45 + 175 * t_ticks/65535;
  tmpHmdBuffer[1] = -6 + 125 * rh_ticks/65535;

  if (rh_pRH > 100) {
    rh_pRH = 100; 
  }
  if (rh_pRH < 0) {
    rh_pRH = 0;
  }

}



void MX_I2C4_Init(void)
{

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
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}



