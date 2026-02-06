#include "TempHumiditySensor.h"

I2C_HandleTypeDef hi2c4;
SemaphoreHandle_t I2C_complete;
StaticSemaphore_t I2C_complete_buffer;


static uint8_t pollCMD = 0xFD;

// polls sensor for temp hmd information
static void sensorPoll(uint8_t* rx_bytes) {

  // transmit data
   if (HAL_I2C_Master_Transmit_IT(&hi2c4, tmpHmdAdresss, &pollCMD, tx_size) != HAL_OK) {
    Error_Handler();
   }

  // wait for interrupt to indicate I2C has been sent
  if (xSemaphoreTake(I2C_complete, pdMS_TO_TICKS(I2C_TIMEOUT)) != pdTRUE) {
    Error_Handler();
  }

  // wait for sensor to sense
  vTaskDelay(pdMS_TO_TICKS(10));

  // recieve data
  if (HAL_I2C_Master_Receive_IT(&hi2c4, tmpHmdAdresss, rx_bytes, rx_size) != HAL_OK) {
    Error_Handler();
  }

  // wait for interrupt to indicate I2C has been recieved.
  if (xSemaphoreTake(I2C_complete, pdMS_TO_TICKS(I2C_TIMEOUT)) != pdTRUE) {
    Error_Handler();
  }
}

// read temperature and humidity, store them in the tmpHmdBuffer passed in as an argument  
void tmpHmd_get(uint16_t *tmpHmdBuffer) {
  
  

  uint8_t rx_bytes[6];
  sensorPoll(rx_bytes);

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
}

// I2C callback function, serve to indicate when I2C communication is complete. Need to have higher level stuff to route this
void SHT4x_I2C_MasterTxRxCpltCallback() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(I2C_complete, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void tmpHmd_init(void)
{

  // TODO: Make code to check if ts (this sensor) works (i.e. check model, status, and others!)

}



