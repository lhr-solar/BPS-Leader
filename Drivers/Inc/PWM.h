#ifndef PWM_H
#define PWM_H

//figure out #includes
#include <stdint.h>
#include "stm32xx_hal.h"

typedef struct{
    TIM_HandleTypeDef* timHandle; 
    uint8_t channel;
    uint64_t speed;
    uint32_t dutyCycle; 
} PWM_Info;


/**
 * @brief Creates queues for timer and initializes TIM for PWM
 * 
 * @param timer handle
 * @return HAL status
 */
HAL_StatusTypeDef PWM_TIM_Init(TIM_HandleTypeDef* timHandle);

/**
 * @brief Configures channel and start 0% duty cycle
 * 
 * @param timer handle, channel
 * @return HAL status
 */
HAL_StatusTypeDef PWM_Channel_Init(TIM_HandleTypeDef* timHandle, uint8_t channel);

/**
 * @brief Adds PWM request to queue and enables interrupts if needed
 * 
 * @param timer handle, channel, duty cycle
 * @return HAL status
 */
HAL_StatusTypeDef PWM_Set(TIM_HandleTypeDef* timHandle, uint8_t channel, uint32_t dutyCycle);

/**
 * @brief Called within PeriodElapsedCallback and pops from queue and changes PWM duty cycle,
 *        Disables interrupts if queue is empty
 * 
 * @param timer handle
 * @return HAL status
 */
void PWM_PeriodElapsed(TIM_HandleTypeDef *timHandle);

#endif