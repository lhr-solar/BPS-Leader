
#include "DebugPrintf.h"
#include <stdio.h>

void HAL_UART_MspGPIOInit(UART_HandleTypeDef *huart) {

    GPIO_InitTypeDef init = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    init.Pin = GPIO_PIN_10|GPIO_PIN_11;
    init.Mode = GPIO_MODE_AF_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    init.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOC, &init);

}

void debugPrintf_init(){
    
    HAL_UART_MspGPIOInit(husart3);

    husart3->Init.BaudRate = 115200;
    husart3->Init.WordLength = UART_WORDLENGTH_8B;
    husart3->Init.StopBits = UART_STOPBITS_1;
    husart3->Init.Parity = UART_PARITY_NONE;
    husart3->Init.Mode = UART_MODE_TX_RX;
    husart3->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    husart3->Init.OverSampling = UART_OVERSAMPLING_16;

    setvbuf(stdout, NULL, _IONBF, 0);

    printf_init(husart3);
}


