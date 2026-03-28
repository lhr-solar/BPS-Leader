#pragma once

#include "common.h"
#include "UART.h"
#include "ADC.h"
#include "ADC_Battery_LUT.h"
#include "ADC_Array_LUT.h"

// ADC Channel 11 (Array Voltage) and 12 (Battery Voltage) are GPIOB 12 and 2
#define ADC1_CHANNEL ADC_CHANNEL_11
#define ADC2_CHANNEL ADC_CHANNEL_12
#define ADC_QUEUE_LENGTH 4
#define ADC_QUEUE_ITEM_SIZE sizeof(uint16_t)
#define ADC_SAMPLING_TIME ADC_SAMPLETIME_2CYCLES_5

/**
 * @brief ADC voltage measurement results
 *
 * Contains scaled array and battery voltages in millivolts.
 */
typedef struct
{
  uint32_t Array_Voltage;
  uint32_t Battery_Voltage;
} ADC_Sense_Result;

/**
 * @brief ADC sense function return status
 */
typedef enum
{
  ADC_SENSE_OK = 0,         // Operation successful
  ADC_QUEUE_ERR,            // Queue creation failed
  ADC_1_INIT_ERR,           // ADC1 initialization failed
  ADC_2_INIT_ERR,           // ADC2 initialization failed
  ADC_1_READ_ERR,           // ADC1 read failed
  ADC_2_READ_ERR,           // ADC2 read failed
  ADC_SENSE_INIT_ERR,       // Initialization not called or failed
  READ_ADC_BAD_PARAM_ERR,   // Bad result parameter
  ARRAY_QUEUE_RECEIVE_ERR,  // ADC values not received from array ADC queue
  BATTERY_QUEUE_RECEIVE_ERR // ADC values not received from battery ADC queue
} ADC_Sense_Status_t;

/**
 * @brief   Initialize ADC peripherals and internal queues
 *
 * Creates ADC queues and initializes both ADC instances.
 *
 * @return  ADC_SENSE_OK if successful, ADC_SENSE_ERR otherwise
 */
ADC_Sense_Status_t ADC_Sense_Init(void);

/**
 * @brief   Read ADC values and compute scaled voltages
 *
 * Triggers ADC conversions, waits for samples, and converts raw ADC
 * counts into millivolt values using fixed-point math.
 *
 * @param   Timeout_MS  Maximum time to wait for ADC samples in milliseconds
 * @param   Result      Pointer to result structure for voltages
 * @return  ADC_SENSE_OK if both ADC channels updated successfully,
 *          ADC_SENSE_ERR otherwise
 */
ADC_Sense_Status_t Read_ADC(TickType_t Timeout_MS, ADC_Sense_Result *Result);


