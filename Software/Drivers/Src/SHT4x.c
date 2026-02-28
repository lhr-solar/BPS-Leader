#include "SHT4x.h"

#define SENSE_WAIT 10

I2C_HandleTypeDef hi2c4;
static SemaphoreHandle_t I2C_complete;
static StaticSemaphore_t I2C_complete_buffer;

static uint8_t pollCMD = 0xFD;

// polls sensor for temp hmd information
static SHT45_status_t sensorPoll(uint8_t* rx_bytes) {

  // transmit data
   if (HAL_I2C_Master_Transmit_IT(&hi2c4, tmpHmdAdresss, &pollCMD, tx_size) != HAL_OK) {
    return SHT45_ERR;
   }

  // wait for interrupt to indicate I2C has been sent
  if (xSemaphoreTake(I2C_complete, pdMS_TO_TICKS(I2C_TIMEOUT)) != pdTRUE) {
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
SHT45_status_t tmpHmd_get(uint32_t *tmpHmdBuffer) {

  uint8_t rx_bytes[6];
  if (sensorPoll(rx_bytes) != SHT45_OK) {
      return SHT45_ERR;
  }

  int32_t t_ticks = rx_bytes[0] * 256 + rx_bytes[1];
  int32_t rh_ticks = rx_bytes[3] * 256 + rx_bytes[4];
  tmpHmdBuffer[0] = -45 + 175 * t_ticks/65535;
  tmpHmdBuffer[1] = -6 + 125 * rh_ticks/65535;

  if (tmpHmdBuffer[1] > 100) {
    tmpHmdBuffer[1] = 100; 
  }
  if (tmpHmdBuffer[0] < 0) {
    tmpHmdBuffer[0] = 0;
  }

  return SHT45_OK;
}

// I2C callback function, serve to indicate when I2C communication is complete. Need to have higher level stuff to route this
void SHT4x_I2C_MasterTxRxCpltCallback() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(I2C_complete, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void SHT4x_init() {

  I2C_complete = xSemaphoreCreateMutexStatic(&I2C_complete_buffer);

}




