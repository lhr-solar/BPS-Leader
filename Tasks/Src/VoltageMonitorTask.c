#include "BPS_Tasks.h"
#include "stm32xx_hal.h"

void Task_Voltage_Monitor(){

    while(1){
        
        // Delays 5 ms
        vTaskDelay(5);
    }
    
}


#include "BPS_Tasks.h"
#include "stm32xx_hal.h"

// External ADC handle (declare according to your configuration)
extern ADC_HandleTypeDef hadc1;

// Reference voltage and ADC resolution (adjust for your hardware)
#define VREF_VOLTAGE    3.3f
#define ADC_RESOLUTION  4095.0f  // For 12-bit ADC

void Task_Voltage_Monitor() {
    while(1) {
        float voltages[3];  // Store readings for 3 channels
        ADC_ChannelConfTypeDef sConfig = {0};

        // Read Channel 0 (VBAT)
        sConfig.Channel = ADC_CHANNEL_0;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        
        HAL_ADC_Start(&hadc1);
        if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            voltages[0] = (HAL_ADC_GetValue(&hadc1) * VREF_VOLTAGE) / ADC_RESOLUTION;
        }

        // Read Channel 1 (VBus)
        sConfig.Channel = ADC_CHANNEL_1;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        
        HAL_ADC_Start(&hadc1);
        if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            voltages[1] = (HAL_ADC_GetValue(&hadc1) * VREF_VOLTAGE) / ADC_RESOLUTION;
        }

        // Read Channel 2 (VEXT)
        sConfig.Channel = ADC_CHANNEL_2;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        
        HAL_ADC_Start(&hadc1);
        if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            voltages[2] = (HAL_ADC_GetValue(&hadc1) * VREF_VOLTAGE) / ADC_RESOLUTION;
        }

        // Add your voltage processing/logic here
        // (e.g., send over CAN, check thresholds, etc.)

        vTaskDelay(pdMS_TO_TICKS(5));  // FreeRTOS delay
    }
}