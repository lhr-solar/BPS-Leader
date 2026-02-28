#include "common.h"
#include "ADC.h"
#include "pindef.h"
#include "ADC_Driver.h"
#include "UART.h"
#include "DebugPrintf.h"

static uint8_t Is_Initialized = 0;
static uint32_t Error_Mask = ADC_SENSE_ERR_NONE;

ADC_InitTypeDef adc_init_1 = {0};
ADC_InitTypeDef adc_init_2 = {0};

QueueHandle_t Motor_ADC_Queue;
QueueHandle_t Battery_ADC_Queue;

static StaticQueue_t xStaticQueue1;
static StaticQueue_t xStaticQueue2;
uint8_t qStorage1[ADC_QUEUE_LENGTH * ITEM_SIZE];
uint8_t qStorage2[ADC_QUEUE_LENGTH * ITEM_SIZE];

static ADC_ChannelConfTypeDef sConfig1 = {
    .Channel = ADC_CHANNEL_11,
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};

static ADC_ChannelConfTypeDef sConfig2 = {
    .Channel = ADC_CHANNEL_12,
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};

ADC_Sense_Status ADC_Sense_Init(void) // Initialize ADCs and queues
{
    ADC_MultiModeTypeDef multimode = {0};

    Motor_ADC_Queue = xQueueCreateStatic(ADC_QUEUE_LENGTH, ITEM_SIZE, qStorage1, &xStaticQueue1);
    Battery_ADC_Queue = xQueueCreateStatic(ADC_QUEUE_LENGTH, ITEM_SIZE, qStorage2, &xStaticQueue2);

    Is_Initialized = 0;

    if (Motor_ADC_Queue == NULL || Battery_ADC_Queue == NULL)
    {
        Error_Mask |= ADC_SENSE_ERR_NOT_INIT;
        return ADC_SENSE_ERR_0;
    }

    ADC_InitTypeDef adc_init_1 = {0};
    adc_init_1.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    adc_init_1.Resolution = ADC_RESOLUTION_12B;
    adc_init_1.DataAlign = ADC_DATAALIGN_RIGHT;
    adc_init_1.ScanConvMode = ADC_SCAN_DISABLE;
    adc_init_1.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc_init_1.LowPowerAutoWait = DISABLE;
    adc_init_1.ContinuousConvMode = DISABLE;
    adc_init_1.NbrOfConversion = 1;
    adc_init_1.DiscontinuousConvMode = DISABLE;
    adc_init_1.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_init_1.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc_init_1.DMAContinuousRequests = DISABLE;
    adc_init_1.Overrun = ADC_OVR_DATA_PRESERVED;
    adc_init_1.OversamplingMode = DISABLE;

    if (adc_init(&adc_init_1, hadc1) != ADC_OK)
    {
        Error_Mask |= ADC_SENSE_ERR_ADC1_INIT;
        return ADC_SENSE_ERR_1;
    }

      multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(hadc1, &multimode) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADC_ConfigChannel(hadc1, &sConfig1) != HAL_OK)
    {
        Error_Handler();
    }

    ADC_InitTypeDef adc_init_2 = {0};
    adc_init_2.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    adc_init_2.Resolution = ADC_RESOLUTION_12B;
    adc_init_2.DataAlign = ADC_DATAALIGN_RIGHT;
    adc_init_2.ScanConvMode = ADC_SCAN_DISABLE;
    adc_init_2.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc_init_2.LowPowerAutoWait = DISABLE;
    adc_init_2.ContinuousConvMode = DISABLE;
    adc_init_2.NbrOfConversion = 1;
    adc_init_2.DiscontinuousConvMode = DISABLE;
    adc_init_2.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_init_2.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc_init_2.DMAContinuousRequests = DISABLE;
    adc_init_2.Overrun = ADC_OVR_DATA_PRESERVED;
    adc_init_2.OversamplingMode = DISABLE;

    if (adc_init(&adc_init_2, hadc2) != ADC_OK)
    {
        Error_Mask |= ADC_SENSE_ERR_ADC2_INIT;
        return ADC_SENSE_ERR_2;
    }

    if (HAL_ADC_ConfigChannel(hadc2, &sConfig2) != HAL_OK)
    {
        Error_Handler();
    }

    

    Is_Initialized = 1;
    return ADC_SENSE_OK;
}

uint32_t ADC_Sense_GetErrorMask(void)
{
    return Error_Mask;
}

void ADC_Sense_ClearErrors(uint32_t Mask)
{
    Error_Mask &= ~Mask;
}

ADC_Sense_Status Read_ADC(uint32_t Timeout_MS, ADC_Sense_Result *Result, uint32_t *Updated_Mask) // Read ADC values and calculate voltages
{
    if (!Is_Initialized)
    {
        if (Updated_Mask)
            *Updated_Mask = ADC_SENSE_UPD_NONE;
        Error_Mask |= ADC_SENSE_ERR_NOT_INIT;
        return ADC_SENSE_ERR;
    }

    if (Result == NULL)
    {
        Error_Mask |= ADC_SENSE_ERR_BAD_PARAM;
        return ADC_SENSE_ERR;
    }

    uint16_t Motor_ADC = 0;
    uint16_t Battery_ADC = 0;
    uint32_t Updated = ADC_SENSE_UPD_NONE;
    TickType_t Timeout_Ticks = pdMS_TO_TICKS(Timeout_MS);

    HAL_ADCEx_Calibration_Start(hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(hadc2, ADC_SINGLE_ENDED);

    adc_status_t adc1_status = adc_read(hadc1, &sConfig1, Motor_ADC_Queue);
    adc_status_t adc2_status = adc_read(hadc2, &sConfig2, Battery_ADC_Queue);
    
    if (adc1_status != ADC_OK)
    {
        return ADC_SENSE_ERR;
    }
    if (adc2_status != ADC_OK)
    {
        return ADC_SENSE_ERR;
    }

    if (xQueueReceive(Motor_ADC_Queue, &Motor_ADC, Timeout_Ticks) == pdPASS)
    {
        uint32_t raw = ADC1->DR;
        (void)raw;
        int64_t Numerator = (int64_t)Motor_ADC * V_Ref * Gain_Denominator * Divider_Denominator; // Convert ADC reading to voltage in mV with scaling factors
        int64_t Denominator = (int64_t)ADC_Max * Gain_Numerator * Divider_Numerator;
        Result->Motor_Voltage = (int32_t)(Numerator / Denominator);
        Updated |= ADC_SENSE_UPD_MOTOR;
    }
    else
    {
        // Motor ADC stopped
        Error_Mask |= ADC_SENSE_ERR_MOTOR_STALE;
    }

    if (xQueueReceive(Battery_ADC_Queue, &Battery_ADC, Timeout_Ticks) == pdPASS)
    {
        int64_t Numerator = (int64_t)Battery_ADC * V_Ref * Gain_Denominator * Divider_Denominator; // Convert ADC reading to voltage in mV with scaling factors
        int64_t Denominator = (int64_t)ADC_Max * Gain_Numerator * Divider_Numerator;
        Result->Battery_Voltage = (int32_t)(Numerator / Denominator);
        Updated |= ADC_SENSE_UPD_BATTERY;
    }
    else
    {
        // Battery ADC stopped
        Error_Mask |= ADC_SENSE_ERR_BATT_STALE;
    }

    if (Updated_Mask)
    {
        *Updated_Mask = Updated;
    }

    if ((Updated & (ADC_SENSE_UPD_MOTOR | ADC_SENSE_UPD_BATTERY)) != (ADC_SENSE_UPD_MOTOR | ADC_SENSE_UPD_BATTERY)) // If either ADC failed to update, return error
    {
        return ADC_SENSE_ERR;
    }

    return ADC_SENSE_OK;
}

