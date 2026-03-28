#include "ADC_Driver.h"
#include "config.h"

static uint8_t Is_Initialized = 0;

ADC_InitTypeDef adc_init_1 = {0};
ADC_InitTypeDef adc_init_2 = {0};

QueueHandle_t Array_ADC_Queue;
QueueHandle_t Battery_ADC_Queue;

static StaticQueue_t xStaticQueue1;
static StaticQueue_t xStaticQueue2;
uint8_t qStorage1[ADC_QUEUE_LENGTH * ADC_QUEUE_ITEM_SIZE];
uint8_t qStorage2[ADC_QUEUE_LENGTH * ADC_QUEUE_ITEM_SIZE];

// This is for Array ADC channel
static ADC_ChannelConfTypeDef sConfig1 = {
    .Channel = ADC1_CHANNEL,
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLING_TIME,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0};

// This is for Battery ADC channel
static ADC_ChannelConfTypeDef sConfig2 = {
    .Channel = ADC2_CHANNEL,
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLING_TIME,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0};

static uint32_t HAL_RCC_ADC12_CLK_ENABLED = 0;

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    if (adcHandle->Instance == ADC1)
    {
        /** Initializes the peripherals clocks
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
        PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        /* ADC1 clock enable */
        HAL_RCC_ADC12_CLK_ENABLED++;
        if (HAL_RCC_ADC12_CLK_ENABLED == 1)
        {
            __HAL_RCC_ADC12_CLK_ENABLE();
        }

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**ADC1 GPIO Configuration
        PB12     ------> ADC1_IN11
        */
        GPIO_InitStruct.Pin = ADC1_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ADC_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(ADC1_2_IRQn, ADC_IRQ_PRIO, 0);
        HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
    }
    else if (adcHandle->Instance == ADC2)
    {
        /** Initializes the peripherals clocks
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
        PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        /* ADC2 clock enable */
        HAL_RCC_ADC12_CLK_ENABLED++;
        if (HAL_RCC_ADC12_CLK_ENABLED == 1)
        {
            __HAL_RCC_ADC12_CLK_ENABLE();
        }

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**ADC2 GPIO Configuration
        PB2     ------> ADC2_IN12
        */
        GPIO_InitStruct.Pin = ADC2_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ADC_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(ADC1_2_IRQn, ADC_IRQ_PRIO, 0);
        HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
    }
}

// Init for Array ADC channel
ADC_Sense_Status_t ADC_1_Init()
{
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
        // ADC1 initialization failed
        return ADC_1_INIT_ERR;
    }

    ADC_MultiModeTypeDef multimode = {0};
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(hadc1, &multimode) != HAL_OK)
    {
        return ADC_1_INIT_ERR;
    }

    if (HAL_ADC_ConfigChannel(hadc1, &sConfig1) != HAL_OK)
    {
        return ADC_1_INIT_ERR;
    }

    HAL_ADCEx_Calibration_Start(hadc1, ADC_SINGLE_ENDED);

    return ADC_SENSE_OK;
}


// Init for Battery ADC channel
ADC_Sense_Status_t ADC_2_Init()
{
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
        // ADC2 initialization failed
        return ADC_2_INIT_ERR;
    }

    if (HAL_ADC_ConfigChannel(hadc2, &sConfig2) != HAL_OK)
    {
        return ADC_2_INIT_ERR;
    }

    HAL_ADCEx_Calibration_Start(hadc2, ADC_SINGLE_ENDED);

    return ADC_SENSE_OK;
}

ADC_Sense_Status_t ADC_Sense_Init(void) // Initialize ADCs and queues
{
    Array_ADC_Queue = xQueueCreateStatic(ADC_QUEUE_LENGTH, ADC_QUEUE_ITEM_SIZE, qStorage1, &xStaticQueue1);
    Battery_ADC_Queue = xQueueCreateStatic(ADC_QUEUE_LENGTH, ADC_QUEUE_ITEM_SIZE, qStorage2, &xStaticQueue2);

    Is_Initialized = 0;

    if (Array_ADC_Queue == NULL || Battery_ADC_Queue == NULL)
    {
        // Queue creation failed
        return ADC_QUEUE_ERR;
    }

    if (ADC_1_Init() != ADC_SENSE_OK || ADC_2_Init() != ADC_SENSE_OK)
    {
        // One or both ADC initializations failed
        return ADC_SENSE_INIT_ERR;
    }

    Is_Initialized = 1;
    return ADC_SENSE_OK;
}

ADC_Sense_Status_t Read_ADC(TickType_t Timeout_MS, ADC_Sense_Result *Result) // Read ADC values and calculate voltages
{
    if (!Is_Initialized)
    {
        // ADC_Sense_Init has not been called or failed, 
        if (ADC_Sense_Init() != ADC_SENSE_OK) return ADC_SENSE_INIT_ERR;
    }

    if (Result == NULL)
    {
        // Invalid result pointer
        return READ_ADC_BAD_PARAM_ERR;
    }

    uint16_t Array_ADC = 0;
    uint16_t Battery_ADC = 0;
    TickType_t Timeout_Ticks = pdMS_TO_TICKS(Timeout_MS);

    if (adc_read(hadc1, &sConfig1, Array_ADC_Queue) != ADC_OK)
    {
        return ADC_1_READ_ERR;
    }
    if (adc_read(hadc2, &sConfig2, Battery_ADC_Queue) != ADC_OK)
    {
        return ADC_2_READ_ERR;
    }

    if (xQueueReceive(Array_ADC_Queue, &Array_ADC, Timeout_Ticks) == pdPASS)
    {
        Result->Array_Voltage = Array_LUT[Array_ADC];
    }
    else
    {
        // Queue receive failed for array ADC
        return ARRAY_QUEUE_RECEIVE_ERR;
    }

    if (xQueueReceive(Battery_ADC_Queue, &Battery_ADC, Timeout_Ticks) == pdPASS)
    {
        Result->Battery_Voltage = Battery_LUT[Battery_ADC];
    }
    else
    {
        // Queue receive failed fo  r battery ADC
        return BATTERY_QUEUE_RECEIVE_ERR;
    }

    return ADC_SENSE_OK;
}