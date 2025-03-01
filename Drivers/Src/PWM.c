#include "PWM.h"
#include "LEDs.h"

#ifndef PWM_SEND_QUEUE_SIZE
#define PWM_SEND_QUEUE_SIZE (10)
#endif

enum {PWM_INFO_SIZE = sizeof(PWM_Info) };

static QueueHandle_t pwm1_send_queue = NULL; 
static StaticQueue_t pwm1_send_queue_buffer;
static uint8_t pwm1_send_queue_storage[PWM_SEND_QUEUE_SIZE*PWM_INFO_SIZE];
        
static QueueHandle_t pwm2_send_queue = NULL; 
static StaticQueue_t pwm2_send_queue_buffer;
static uint8_t pwm2_send_queue_storage[PWM_SEND_QUEUE_SIZE*PWM_INFO_SIZE];

static TIM_OC_InitTypeDef sConfigOC = {0};

QueueHandle_t* PWM_Get_Queue(TIM_HandleTypeDef* timHandle) {
    switch((uint32_t)timHandle->Instance) {
        case (uint32_t)TIM1:
            return &pwm1_send_queue;
        case (uint32_t)TIM2:
            return &pwm2_send_queue;
        //add more timers later maybe
    }
    return 0;
}

void PWM_IRQ_Enable(TIM_HandleTypeDef* timHandle) {
    switch((uint32_t)timHandle->Instance) {
        case (uint32_t)TIM1:
            break;
        case (uint32_t)TIM2:
            if(!HAL_NVIC_GetActive(TIM2_IRQn))
                HAL_NVIC_EnableIRQ(TIM2_IRQn); 
            break;
        //add more timers later maybes
    }
    
}

void PWM_IRQ_Disable(TIM_HandleTypeDef* timHandle) {
    switch((uint32_t)timHandle->Instance) {
        case (uint32_t)TIM1:
            break;
        case (uint32_t)TIM2:
            HAL_NVIC_DisableIRQ(TIM2_IRQn); break;
        //add more timers later maybes
    }
    
}

HAL_StatusTypeDef PWM_TIM_Init(TIM_HandleTypeDef* timHandle) {
    HAL_StatusTypeDef stat = HAL_OK;
    
    if(__HAL_RCC_GPIOA_IS_CLK_DISABLED())
         __HAL_RCC_GPIOA_CLK_ENABLE();
    
    if (timHandle->Instance == TIM1 && pwm1_send_queue == NULL) {// assign queue for specific timer
        pwm1_send_queue = xQueueCreateStatic(PWM_SEND_QUEUE_SIZE, PWM_INFO_SIZE,
        pwm1_send_queue_storage, &pwm1_send_queue_buffer);
    
        if (pwm1_send_queue == NULL) 
            return HAL_ERROR; 
    }
    else if (timHandle->Instance == TIM2 && pwm2_send_queue == NULL) {
        
        pwm2_send_queue = xQueueCreateStatic(PWM_SEND_QUEUE_SIZE, PWM_INFO_SIZE,
        pwm2_send_queue_storage, &pwm2_send_queue_buffer); 

        if (pwm2_send_queue == NULL) 
            return HAL_ERROR; 
    }

    stat = HAL_TIM_PWM_Init(timHandle);
    if(stat == HAL_ERROR) return stat;

    return stat;
}

HAL_StatusTypeDef PWM_Channel_Init(TIM_HandleTypeDef* timHandle, uint8_t channel) {    
    HAL_StatusTypeDef stat = HAL_OK;
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;  
    sConfigOC.Pulse = 5000;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    stat = HAL_TIM_PWM_ConfigChannel(timHandle, &sConfigOC, channel);
    if(stat == HAL_ERROR) return stat;

    stat = HAL_TIM_PWM_Start(timHandle, channel); 
    return stat;
}

HAL_StatusTypeDef PWM_Set(TIM_HandleTypeDef* timHandle, uint8_t channel, uint8_t dutyCycle, uint64_t speed) {
    
    // HAL_TIM_PWM_Stop(timHandle, channel); 
    // HAL_TIM_PWM_Start_IT(timHandle, channel);
    // PWM_IRQ_Enable(timHandle);
    if(dutyCycle > 100) return HAL_ERROR;

    QueueHandle_t tx_Queue = *PWM_Get_Queue(timHandle);
    
    portENTER_CRITICAL();
    PWM_Info pwmSend = { // for storing PWM info into queue
        .timHandle = timHandle,
        .channel = channel,
        .dutyCycle = dutyCycle,
        .speed = speed
    };
    portEXIT_CRITICAL();

    if(!xQueueSend(tx_Queue, &pwmSend, 0)) {
        return HAL_ERROR;
    }

    return HAL_OK;
    
}

void PWM_PeriodElapsed(TIM_HandleTypeDef *timHandle) {
    
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    QueueHandle_t tx_Queue = *PWM_Get_Queue(timHandle);
    

    if(tx_Queue) {
        if(!xQueueIsQueueEmptyFromISR(tx_Queue)) {
            PWM_Info pwmReceive; 
            xQueueReceiveFromISR(tx_Queue, &pwmReceive, &higherPriorityTaskWoken);
            // HAL_TIM_PWM_Stop(pwmReceive.timHandle, pwmReceive.channel);
            HAL_TIM_PWM_Stop(pwmReceive.timHandle, pwmReceive.channel);
            sConfigOC.Pulse = (pwmReceive.dutyCycle*pwmReceive.timHandle->Init.Period)/100;
            // portENTER_CRITICAL();
            HAL_TIM_PWM_ConfigChannel(pwmReceive.timHandle, &sConfigOC, pwmReceive.channel);
            HAL_TIM_PWM_Start(pwmReceive.timHandle, pwmReceive.channel);
            // portEXIT_CRITICAL();
        } else {
            // PWM_IRQ_Disable(timHandle);
        }
    }

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}