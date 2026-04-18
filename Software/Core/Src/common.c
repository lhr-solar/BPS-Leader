#include "Contactors.h"
#include "StatusLEDs.h"
#include "common.h"

// delays if scheduler is started or not
#define safe_delay(ms) ((xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) ? vTaskDelay(pdMS_TO_TICKS(ms)) : HAL_Delay(ms))

// checks to make sure scheduler is started before printint
#define safe_printf(str) ((xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) ? printf(str) : 0) 

// for software errors
void Error_Handler() {

    // open every contactor, bypasses semaphore
    emergency_open_contactors();
    
    // turns on fault led, and blink DEBUG led to show the error was software
    setHeartbeat(LED_OFF);
    LED_set(FAULT_LED, LED_ON);
    while (true) {
        LED_set(DEBUG_LED, LED_ON);
        safe_delay(1000);
        LED_set(DEBUG_LED, LED_OFF);
        safe_delay(1000);
        safe_printf("Faulted\n\r");
    }
}


TickType_t Calculate_TimeDifference(TickType_t newTime, TickType_t oldTime)
{
    // Normal case: The timer has not wrapped around
    if (newTime >= oldTime) 
    {
        return newTime - oldTime;
    } 
    // Overflow case: The timer wrapped around past its maximum value
    else 
    {
        // 1. Find distance from oldTime to the absolute maximum tick value
        // 2. Add the newTime (distance from 0)
        // 3. Add 1 to account for the actual zero-crossing
        // Note: ~((TickType_t)0) automatically resolves to the max value of the type (e.g. 0xFFFFFFFF)
        return (~((TickType_t)0) - oldTime) + newTime + 1;
    }
}

    void SystemClock_Config(void)
{

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


    /** Configure the main internal regulator output voltage
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 20;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}