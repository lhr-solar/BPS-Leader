#include "PWM.h"

#ifndef PWM_SEND_QUEUE_SIZE
#define PWM_SEND_QUEUE_SIZE (10)
#endif

enum {PWM_INFO_SIZE = sizeof(PWM_Info) };
BaseType_t higherPriorityTaskWoken = pdFALSE;

static QueueHandle_t pwm1_send_queue = NULL;  // need to add queue for each PWM timer
static StaticQueue_t pwm1_send_queue_buffer;
static uint8_t pwm1_send_queue_storage[PWM_SEND_QUEUE_SIZE*PWM_INFO_SIZE];
        
static QueueHandle_t pwm2_send_queue = NULL;  // need to add queue for each PWM timer
static StaticQueue_t pwm2_send_queue_buffer;
static uint8_t pwm2_send_queue_storage[PWM_SEND_QUEUE_SIZE*PWM_INFO_SIZE];

static TIM_OC_InitTypeDef sConfigOC = {0};

void MX_GPIO_Init(){
    GPIO_InitTypeDef pwm_tim1_ch1 = {
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_8,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF1_TIM1
    };

    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5
    };
    
    // GPIO_InitTypeDef pwm_tim1_ch2 = {
    //     .Mode = GPIO_MODE_AF_PP,
    //     .Pull = GPIO_NOPULL,
    //     .Pin = GPIO_PIN_1,
    //     .Speed = GPIO_SPEED_FREQ_LOW,
    //     .Alternate = GPIO_AF1_TIM1
    // };

    // GPIO_InitTypeDef pwm_tim2_ch1 = {
    //     .Mode = GPIO_MODE_AF_PP,
    //     .Pull = GPIO_NOPULL,
    //     .Pin = GPIO_PIN_0,
    //     .Speed = GPIO_SPEED_FREQ_LOW,
    //     .Alternate = GPIO_AF1_TIM2
    // };
    
    GPIO_InitTypeDef pwm_tim9_ch1 = {
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_2,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF3_TIM9
    };

    HAL_GPIO_Init(GPIOA, &pwm_tim1_ch1);
    HAL_GPIO_Init(GPIOA, &led_config);
    HAL_GPIO_Init(GPIOA, &pwm_tim9_ch1);
}

QueueHandle_t* PWM_Get_Queue(TIM_HandleTypeDef* timHandle) {
    switch((uint32_t)timHandle->Instance) {
        case (uint32_t)TIM1:
            return &pwm1_send_queue;
        case (uint32_t)TIM9:
            return &pwm2_send_queue;
        //add more timers later maybe
    }
    return 0;
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
    else if (timHandle->Instance == TIM9 && pwm2_send_queue == NULL) {
        
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

    stat = HAL_TIM_PWM_Start_IT(timHandle, channel); 
    return stat;
}

HAL_StatusTypeDef PWM_Set(TIM_HandleTypeDef* timHandle, uint8_t channel, uint8_t dutyCycle, uint64_t speed) {
    if(dutyCycle > 100) return HAL_ERROR;

    QueueHandle_t* tx_Queue = BSP_PWM_Get_Queue(timHandle);
    
    PWM_Info pwmSend = { // for storing PWM info into queue
        .timHandle = timHandle,
        .channel = channel,
        .dutyCycle = dutyCycle,
        .speed = speed
    };

    if(!xQueueSend(*tx_Queue, &pwmSend, 0)) {
        return HAL_ERROR;
    }
            
    return HAL_OK;
    
}

void HAL_TIM_PWM_PeriodElapsedCallback(TIM_HandleTypeDef *timHandle) {

    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // testing interrupt callback
    QueueHandle_t* tx_Queue = BSP_PWM_Get_Queue(timHandle);
 

    if(!xQueueIsQueueEmptyFromISR(*tx_Queue)) {
        PWM_Info pwmReceive; 
        xQueueReceiveFromISR(*tx_Queue, &pwmReceive, &higherPriorityTaskWoken);
        HAL_TIM_PWM_Stop_IT(pwmReceive.timHandle, pwmReceive.channel);
        sConfigOC.Pulse = (pwmReceive.dutyCycle*pwmReceive.timHandle->Init.Period)/100;
        HAL_TIM_PWM_ConfigChannel(pwmReceive.timHandle, &sConfigOC, pwmReceive.channel);
        HAL_TIM_PWM_Start_IT(pwmReceive.timHandle, pwmReceive.channel);
    }
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}