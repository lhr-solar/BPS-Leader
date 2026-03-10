#include "SHT45.h"

#define SENSE_WAIT 10

I2C_HandleTypeDef hi2c4;

static SemaphoreHandle_t I2C_complete;
static StaticSemaphore_t I2C_complete_buffer;

uint8_t pollCMD = 0xFD;

// polls sensor for temp hmd information
static SHT45_status_t sensorPoll(uint8_t* rx_bytes) {

  // transmit data
   if (HAL_I2C_Master_Transmit_IT(&hi2c4, tmpHmdAdresss, &pollCMD, tx_size) != HAL_OK) {
    return SHT45_ERR;
   }

  // wait for interrupt to indicate I2C has been sent
  if (xSemaphoreTake(I2C_complete, portMAX_DELAY) != pdTRUE) {
    return SHT45_ERR;
  }

  // wait for sensor to sense
  vTaskDelay(pdMS_TO_TICKS(SENSE_WAIT));

  // recieve data
  if (HAL_I2C_Master_Receive_IT(&hi2c4, tmpHmdAdresss, rx_bytes, rx_size) != HAL_OK) {
    return SHT45_ERR;
  }

  // wait for interrupt to indicate I2C has been recieved.
  if (xSemaphoreTake(I2C_complete, pdMS_TO_TICKS(I2C_TIMEOUT)) != pdTRUE) {
    return SHT45_ERR;
  }
  return SHT45_OK; 
}

// read temperature and humidity, store them in the tmpHmdBuffer passed in as an argument  
SHT45_status_t SHT45_get(uint32_t *tmpHmdBuffer) {

  uint8_t rx_bytes[6];
  if (sensorPoll(rx_bytes) != SHT45_OK) {
      return SHT45_ERR;
  }

  int32_t t_ticks = rx_bytes[TEMP] * 256 + rx_bytes[1];
  int32_t rh_ticks = rx_bytes[3] * 256 + rx_bytes[4];
  tmpHmdBuffer[TEMP] = -45 + 175 * t_ticks/65535;
  tmpHmdBuffer[HUMIDITY] = -6 + 125 * rh_ticks/65535;

  if (tmpHmdBuffer[HUMIDITY] > 100) {
    tmpHmdBuffer[HUMIDITY] = 100; 
  }
  if (tmpHmdBuffer[TEMP] < 0) {
    tmpHmdBuffer[TEMP] = 0;
  }

  return SHT45_OK;
}

// I2C callback function, serve to indicate when I2C communication is complete. Need to have higher level stuff to route this
void SHT45_I2C_MasterTxRxCpltCallback() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(I2C_complete, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void SHT45_init(void)
{

  GPIO_InitTypeDef init = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
  PeriphClkInit.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_RCC_I2C4_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  

  init.Pin = GPIO_PIN_6|GPIO_PIN_7;
  init.Mode = GPIO_MODE_AF_OD;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  init.Alternate = GPIO_AF8_I2C4;
  HAL_GPIO_Init(GPIOC, &init);

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

  HAL_NVIC_SetPriority(I2C4_EV_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3, 0);
  HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);

  HAL_NVIC_SetPriority(I2C4_ER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3, 0);
  HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);

  I2C_complete = xSemaphoreCreateBinaryStatic(&I2C_complete_buffer);

}





