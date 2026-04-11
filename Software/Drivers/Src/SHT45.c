

#include "SHT45.h"
#include "config.h"

static SemaphoreHandle_t I2C_complete = NULL;
static StaticSemaphore_t I2C_complete_buffer;

static SemaphoreHandle_t sensor_poll_mutex = NULL;
static StaticSemaphore_t sensor_poll_buffer;

static uint8_t rx_bytes[I2C_RX_SIZE];

bool is_initialized = false;

I2C_HandleTypeDef hi2c4;

static uint8_t pollCMD = 0xFD;

// polls sensor for temp hmd information
static SHT45_status_t SHT45_poll_sensor(uint8_t* rx_bytes, TickType_t delay_ms) {

  // transmit data
   if (HAL_I2C_Master_Transmit_IT(&hi2c4, tmpHmdAdresss, &pollCMD, I2C_TX_SIZE) != HAL_OK) {
    return SHT45_ERR;
   }

  // wait for interrupt to indicate I2C has been sent
  if (xSemaphoreTake(I2C_complete, pdMS_TO_TICKS(delay_ms)) != pdTRUE) {
    return SHT45_ERR;
  }

  // TODO: Figure out why this is load bearing
  vTaskDelay(pdMS_TO_TICKS(10));

  // recieve data
  if (HAL_I2C_Master_Receive_IT(&hi2c4, tmpHmdAdresss, rx_bytes, I2C_RX_SIZE) != HAL_OK) {
    return SHT45_ERR;
  }

  // wait for interrupt to indicate I2C has been recieved.
  if (xSemaphoreTake(I2C_complete, pdMS_TO_TICKS(delay_ms)) != pdTRUE) {
    return SHT45_ERR;
  }
  return SHT45_OK; 
}

// read temperature and humidity, store them in the tmpHmdBuffer passed in as an argument  
SHT45_status_t SHT45_get(int32_t *tmpHmdBuffer, TickType_t delay_ms) {

  if (SHT45_poll_sensor(rx_bytes, delay_ms) != SHT45_OK) {
      return SHT45_ERR;
  }

  int32_t t_ticks = rx_bytes[0] * 256 + rx_bytes[1];
  int32_t rh_ticks = rx_bytes[3] * 256 + rx_bytes[4];
  tmpHmdBuffer[SHT45_TEMP] = -45 + 175 * t_ticks/65535;
  tmpHmdBuffer[SHT45_HUMIDITY] = -6 + 125 * rh_ticks/65535;

  if (tmpHmdBuffer[SHT45_HUMIDITY] > 100) {
    tmpHmdBuffer[SHT45_HUMIDITY] = 100; 
  }
  if (tmpHmdBuffer[SHT45_TEMP] < 0) {
    tmpHmdBuffer[SHT45_TEMP] = 0;
  }

  return SHT45_OK;
}

// I2C callback function, serve to indicate when I2C communication is complete. Need to have higher level stuff to route this
void SHT45_I2C_MasterTxRxCpltCallback() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (I2C_complete == NULL) {
      set_faultBitFromISR(FAN_CHIP_ERROR, &xHigherPriorityTaskWoken);
  }
  else {
        xSemaphoreGiveFromISR(I2C_complete, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
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

  init.Pin = SHT45_SCL_PIN|SHT45_SDA_PIN;
  init.Mode = GPIO_MODE_AF_OD;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  init.Alternate = GPIO_AF8_I2C4;
  HAL_GPIO_Init(SHT45_SCL_PORT, &init);

  hi2c4.Instance = SHT45_I2C_CHANNEL;
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

  HAL_NVIC_SetPriority(I2C4_EV_IRQn, SHT45_IRQ_PRIO, 0);
  HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);

  HAL_NVIC_SetPriority(I2C4_ER_IRQn, SHT45_IRQ_PRIO, 0);
  HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);

  I2C_complete = xSemaphoreCreateBinaryStatic(&I2C_complete_buffer);
  sensor_poll_mutex = xSemaphoreCreateMutexStatic(&sensor_poll_buffer);

}





