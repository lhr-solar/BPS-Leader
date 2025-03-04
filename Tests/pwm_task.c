// openocd -f openocd-stm32f4x.cfg
// gdb-multiarch Build/stm32f401re.elf
// target extended-remote :3333

#include "stm32xx_hal.h"
#include "PWM.h"
#include "LEDs.h"

#define TASK_INIT_PRIORITY      tskIDLE_PRIORITY + 2
#define TASK_TOGGLE_PRIORITY    tskIDLE_PRIORITY + 2
#define TASK_TIM1_PRIORITY      tskIDLE_PRIORITY + 2
#define TASK_TIM2_PRIORITY      tskIDLE_PRIORITY + 2
#define TASK_TIM3_PRIORITY      tskIDLE_PRIORITY + 2

StaticTask_t Task_Init_Buffer;
StackType_t Task_Init_Stack[configMINIMAL_STACK_SIZE];

StaticTask_t Task_Toggle_Buffer;
StackType_t Task_Toggle_Stack[configMINIMAL_STACK_SIZE];

StaticTask_t Task_TIM1_Buffer;
StackType_t Task_TIM1_Stack[configMINIMAL_STACK_SIZE];

StaticTask_t Task_TIM2_Buffer;
StackType_t Task_TIM2_Stack[configMINIMAL_STACK_SIZE];

StaticTask_t Task_TIM3_Buffer;
StackType_t Task_TIM3_Stack[configMINIMAL_STACK_SIZE];

static TIM_HandleTypeDef tim1;
static TIM_HandleTypeDef tim2;
static TIM_HandleTypeDef tim3;

uint16_t duty = 0;
uint16_t duty2 = 25;
uint16_t duty3 = 0;

static void error_handler(void) {
    while(1) {
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        // HAL_Delay(1000);
    }
}

void Task_Toggle(void * pvParameters) {
    while(1) {
        Heartbeat_Toggle();
        vTaskDelay(200);
    }
}

void Task_TIM1_1(void * pvParameters) {
    while(1) {
        // if(PWM_Set(&tim1, TIM_CHANNEL_1, duty%100, 100000 - 1) != HAL_OK)
            // error_handler();
        duty+=75;
        vTaskDelay(500);
    }
}

void Task_TIM2_1(void * pvParameters) {
    while(1) {
        if(PWM_Set(&tim2, TIM_CHANNEL_1, duty2%100, 100000 - 1) != HAL_OK)
            error_handler();
        duty2+=25;
        vTaskDelay(500);
    }
}

void Task_TIM3_1(void * pvParameters) {
    while(1) {
        if(PWM_Set(&tim3, TIM_CHANNEL_1, duty3%100, 100000 - 1) != HAL_OK)
            error_handler();
        duty3+=50;
        vTaskDelay(500);
    }
}

void GPIO_Init(){
    GPIO_InitTypeDef pwm_tim1_ch1 = {
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_8,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF1_TIM1
    };
    
    GPIO_InitTypeDef pwm_tim2_ch1 = {
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_0,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF1_TIM2
    };

    GPIO_InitTypeDef pwm_tim3_ch1 = {
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_6,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF2_TIM3
    };
    

    // GPIO_InitTypeDef pwm_tim2_ch1 = {
    //     .Mode = GPIO_MODE_AF_PP,
    //     .Pull = GPIO_NOPULL,
    //     .Pin = GPIO_PIN_0,
    //     .Speed = GPIO_SPEED_FREQ_LOW,
    //     .Alternate = GPIO_AF1_TIM2
    // };
    
    // GPIO_InitTypeDef pwm_tim9_ch1 = {
    //     .Mode = GPIO_MODE_AF_PP,
    //     .Pull = GPIO_NOPULL,
    //     .Pin = GPIO_PIN_2,
    //     .Speed = GPIO_SPEED_FREQ_LOW,
    //     .Alternate = GPIO_AF3_TIM9
    // };
    
    Heartbeat_Init();
    HAL_GPIO_Init(GPIOA, &pwm_tim1_ch1);
    HAL_GPIO_Init(GPIOA, &pwm_tim2_ch1);
    HAL_GPIO_Init(GPIOA, &pwm_tim3_ch1);
}

void TIM_Init() {
    tim1.Instance = TIM1;
    tim1.Init.Prescaler = 8-1;
    tim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim1.Init.Period = 10000 - 1;
    tim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    tim2.Instance = TIM2;
    tim2.Init.Prescaler = 8-1;
    tim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim2.Init.Period = 10000 - 1;
    tim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    tim3.Instance = TIM3;
    tim3.Init.Prescaler = 8-1;
    tim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim3.Init.Period = 10000 - 1;
    tim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    __HAL_RCC_TIM2_CLK_ENABLE();
    if (HAL_TIM_Base_Init(&tim2) != HAL_OK) error_handler();
    if (HAL_TIM_Base_Start_IT(&tim2) != HAL_OK) error_handler();
    // HAL_NVIC_EnableIRQ(TIM2_IRQn);
    HAL_NVIC_SetPriority(TIM2_IRQn, 15U, 0U);

    __HAL_RCC_TIM3_CLK_ENABLE();
    if (HAL_TIM_Base_Init(&tim3) != HAL_OK) error_handler();
    if (HAL_TIM_Base_Start_IT(&tim3) != HAL_OK) error_handler();
    // HAL_NVIC_EnableIRQ(TIM3_IRQn);
    HAL_NVIC_SetPriority(TIM3_IRQn, 15U, 0U);
}

void Task_Init_PWM() {

    GPIO_Init();
    TIM_Init();

    // if (PWM_TIM_Init(&tim1) != HAL_OK) error_handler();
    if (PWM_TIM_Init(&tim2) != HAL_OK) error_handler();
    if (PWM_TIM_Init(&tim3) != HAL_OK) error_handler();
    
    // if (PWM_Channel_Init(&tim1, TIM_CHANNEL_1) != HAL_OK) error_handler();
    if (PWM_Channel_Init(&tim2, TIM_CHANNEL_1) != HAL_OK) error_handler();
    if (PWM_Channel_Init(&tim3, TIM_CHANNEL_1) != HAL_OK) error_handler();

    // while(1);
    xTaskCreateStatic(
        Task_Toggle,
        "Toggle Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        TASK_TOGGLE_PRIORITY, //temporary
        Task_Toggle_Stack,
        &Task_Toggle_Buffer
        );

    xTaskCreateStatic(
        Task_TIM1_1,
        "PWM TIM1 CH1 Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        TASK_TIM1_PRIORITY, //temporary
        Task_TIM1_Stack,
        &Task_TIM1_Buffer
        );
    
    xTaskCreateStatic(
        Task_TIM2_1,
        "PWM TIM2 CH1 Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        TASK_TIM2_PRIORITY, //temporary
        Task_TIM2_Stack,
        &Task_TIM2_Buffer
        );

    xTaskCreateStatic(
        Task_TIM3_1,
        "PWM TIM3 CH1 Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        TASK_TIM3_PRIORITY, //temporary
        Task_TIM3_Stack,
        &Task_TIM3_Buffer
        );

    vTaskDelete(NULL);
}

int main(void) {
    // if (HAL_Init() != HAL_OK) error_handler();
    HAL_Init();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    xTaskCreateStatic(
        Task_Init_PWM,
        "Initialization Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        TASK_INIT_PRIORITY,
        Task_Init_Stack,
        &Task_Init_Buffer
    );

    vTaskStartScheduler();

    while(1) {}
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&tim2);
}

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&tim3);
}