#include "PWM.h"
#include "LEDs.h"

#ifndef PWM_SEND_QUEUE_SIZE
#define PWM_SEND_QUEUE_SIZE (10)
#endif

enum {PWM_INFO_SIZE = sizeof(PWM_Info) };
        
static QueueHandle_t pwm2_send_queue = NULL; 
static StaticQueue_t pwm2_send_queue_buffer;
static uint8_t pwm2_send_queue_storage[PWM_SEND_QUEUE_SIZE*PWM_INFO_SIZE];

static QueueHandle_t pwm3_send_queue = NULL; 
static StaticQueue_t pwm3_send_queue_buffer;
static uint8_t pwm3_send_queue_storage[PWM_SEND_QUEUE_SIZE*PWM_INFO_SIZE];

static TIM_OC_InitTypeDef sConfigOC = {0};

static TIM_HandleTypeDef tim2;
static TIM_HandleTypeDef tim3;

QueueHandle_t* PWM_Get_Queue(TIM_HandleTypeDef* timHandle) {
    switch((uint32_t)timHandle->Instance) {
        case (uint32_t)TIM2:
            return &pwm2_send_queue;
        case (uint32_t)TIM3:
            return &pwm3_send_queue;
        //add more timers later maybe
    }
    return 0;
}

void PWM_IRQ_Enable(TIM_HandleTypeDef* timHandle) {
    switch((uint32_t)timHandle->Instance) {
        case (uint32_t)TIM2:
            if(!HAL_NVIC_GetActive(TIM2_IRQn))
                HAL_NVIC_EnableIRQ(TIM2_IRQn); 
            break;
        case (uint32_t)TIM3:
            if(!HAL_NVIC_GetActive(TIM3_IRQn))
                HAL_NVIC_EnableIRQ(TIM3_IRQn); 
            break;
        //add more timers later maybe
    }
    
}

void PWM_IRQ_Disable(TIM_HandleTypeDef* timHandle) {
    switch((uint32_t)timHandle->Instance) {
        case (uint32_t)TIM2:
            HAL_NVIC_DisableIRQ(TIM2_IRQn); break;
        case (uint32_t)TIM3:
            HAL_NVIC_DisableIRQ(TIM3_IRQn); break;
        //add more timers later maybe
    }
}


HAL_StatusTypeDef PWM_TIM_Init(TIM_HandleTypeDef* timHandle) {
    HAL_StatusTypeDef stat = HAL_OK;
    
    if(__HAL_RCC_GPIOA_IS_CLK_DISABLED())
         __HAL_RCC_GPIOA_CLK_ENABLE();
    
    else if (timHandle->Instance == TIM2 && pwm2_send_queue == NULL) {
        tim2 = *timHandle;
        pwm2_send_queue = xQueueCreateStatic(PWM_SEND_QUEUE_SIZE, PWM_INFO_SIZE,
        pwm2_send_queue_storage, &pwm2_send_queue_buffer); 
        if (pwm2_send_queue == NULL) 
            return HAL_ERROR; 
    }
    else if (timHandle->Instance == TIM3 && pwm3_send_queue == NULL) {
        tim3 = *timHandle;
        pwm3_send_queue = xQueueCreateStatic(PWM_SEND_QUEUE_SIZE, PWM_INFO_SIZE,
        pwm3_send_queue_storage, &pwm3_send_queue_buffer); 
        if (pwm3_send_queue == NULL) 
            return HAL_ERROR; 
    }
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;  
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    stat = HAL_TIM_PWM_Init(timHandle);
    if(stat == HAL_ERROR) return stat;

    return stat;
}

HAL_StatusTypeDef PWM_Channel_Init(TIM_HandleTypeDef* timHandle, uint8_t channel) {    
    HAL_StatusTypeDef stat = HAL_OK;

    stat = HAL_TIM_PWM_ConfigChannel(timHandle, &sConfigOC, channel);
    if(stat == HAL_ERROR) return stat;

    stat = HAL_TIM_PWM_Start(timHandle, channel); 
    return stat;
}

HAL_StatusTypeDef PWM_Set(TIM_HandleTypeDef* timHandle, uint8_t channel, uint32_t dutyCycle, uint64_t speed) {
    
    if(dutyCycle > 100) return HAL_ERROR;

    QueueHandle_t tx_Queue = *PWM_Get_Queue(timHandle);
    if(!tx_Queue) return HAL_ERROR;
    portENTER_CRITICAL();
    PWM_Info pwmSend = { // for storing PWM info into queue
        .timHandle = timHandle,
        .channel = channel,
        .dutyCycle = (dutyCycle*timHandle->Init.Period)/100,
        .speed = speed
    };
    portEXIT_CRITICAL();

    if(!xQueueSend(tx_Queue, &pwmSend, 0)) {
        return HAL_ERROR;
    }

    PWM_IRQ_Enable(timHandle);
    return HAL_OK;
    
}

void PWM_PeriodElapsed(TIM_HandleTypeDef *timHandle) {
    
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    QueueHandle_t tx_Queue = *PWM_Get_Queue(timHandle);
    if(!tx_Queue) return;

    if(tx_Queue) {
        if(!xQueueIsQueueEmptyFromISR(tx_Queue)) {
            PWM_Info pwmReceive; 
            xQueueReceiveFromISR(tx_Queue, &pwmReceive, &higherPriorityTaskWoken);
            __HAL_TIM_SET_COMPARE(pwmReceive.timHandle, pwmReceive.channel, pwmReceive.dutyCycle);
        } else {
            PWM_IRQ_Disable(timHandle);
        }
    }

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&tim2);
}

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&tim3);
}