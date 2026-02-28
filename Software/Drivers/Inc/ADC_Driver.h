#ifndef ADC_SENSE_H
#define ADC_SENSE_H

#include "common.h"

// ADC scaling constants
#define ADC_Max 4095
#define V_Ref 3300  // mV

// Isolated amp gain: 2/5 = 0.4
#define Gain_Numerator   2
#define Gain_Denominator 5

// Divider ratio: Rbottom / (Rbottom + Rtop)
#define Divider_Numerator   2490
#define Divider_Denominator (2490 + 100000)

#define ADC_QUEUE_LENGTH 4
#define ITEM_SIZE sizeof(uint16_t)
#define ADC_SAMPLING_TIME ADC_SAMPLETIME_2CYCLES_5

/**
 * @brief ADC voltage measurement results
 *
 * Contains scaled motor and battery voltages in millivolts.
 */
typedef struct {
    uint32_t Motor_Voltage;
    uint32_t Battery_Voltage;
} ADC_Sense_Result;

/**
 * @brief ADC sense function return status
 */
typedef enum {
    ADC_SENSE_OK = 0,
    ADC_SENSE_ERR,
    ADC_SENSE_ERR_0,
    ADC_SENSE_ERR_1,
    ADC_SENSE_ERR_2,
} ADC_Sense_Status;

/**
 * @brief ADC sense error flags (bitmask)
 *
 * Multiple error flags may be set simultaneously.
 */
typedef enum {
    ADC_SENSE_ERR_NONE         = 0,
    ADC_SENSE_ERR_NOT_INIT     = (1u << 0), // Returns when queues are not intialized properly
    ADC_SENSE_ERR_MOTOR_STALE  = (1u << 1), // Returns when motor ADC stops updating
    ADC_SENSE_ERR_BATT_STALE   = (1u << 2), // Returns when battery ADC stops updating
    ADC_SENSE_ERR_ADC1_INIT    = (1u << 3), // Returns when ADC1 adc_init fails
    ADC_SENSE_ERR_ADC2_INIT    = (1u << 4), // Returns when ADC2 adc_init fails
    ADC_SENSE_ERR_BAD_PARAM    = (1u << 5), // Returns when READ_ADC is called with a NULL Result pointer
} ADC_Sense_ErrorMask;

/**
 * @brief ADC update flags indicating which channels were updated
 */
typedef enum {
    ADC_SENSE_UPD_NONE    = 0,
    ADC_SENSE_UPD_MOTOR   = (1u << 0),
    ADC_SENSE_UPD_BATTERY = (1u << 1),
} ADC_Sense_UpdateMask;

/**
 * @brief   Get the current ADC error mask
 *
 * @return  Bitmask of ADC_SENSE_ERR_* flags
 */
uint32_t ADC_Sense_GetErrorMask(void);

/**
 * @brief   Clear selected ADC error flags
 *
 * @param   Mask Bitmask of error flags to clear
 */
void     ADC_Sense_ClearErrors(uint32_t Mask);

/**
 * @brief   Initialize ADC peripherals and internal queues
 *
 * Creates ADC queues and initializes both ADC instances.
 *
 * @return  ADC_SENSE_OK if successful, ADC_SENSE_ERR otherwise
 */
ADC_Sense_Status ADC_Sense_Init(void);

/**
 * @brief   Read ADC values and compute scaled voltages
 *
 * Triggers ADC conversions, waits for samples, and converts raw ADC
 * counts into millivolt values using fixed-point math.
 *
 * @param   Timeout_MS      Maximum time to wait for ADC samples (ticks)
 * @param   Result       Pointer to result structure for voltages
 * @param   Updated_Mask Optional pointer to update mask output
 * @return  ADC_SENSE_OK if both ADC channels updated successfully,
 *          ADC_SENSE_ERR otherwise
 */
ADC_Sense_Status Read_ADC(uint32_t Timeout_MS,  ADC_Sense_Result *Result, uint32_t *Updated_Mask);

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
void MX_ADC1_Init(void);

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
void MX_ADC2_Init(void);

#endif