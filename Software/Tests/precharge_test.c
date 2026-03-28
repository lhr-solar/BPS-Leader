
#include "PrechargeTask.h"
#include "FaultHandlerTask.h"

int main()
{
    if (HAL_Init() != HAL_OK)
        Error_Handler();
    SystemClock_Config();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    // Task
    hprecharge_task = xTaskCreateStatic(
        Task_Precharge,             // Task function
        "Precharge",                // Name of the task (for debugging)
        configMINIMAL_STACK_SIZE,   // Stack size in words
        NULL,                       // Task input parameter
        tskIDLE_PRIORITY + 1,       // Task priority
        Precharge_Task_Stack,       // Task handle
        &Precharge_Task_Buffer      // Static task buffer (optional)
    );

    xTaskCreateStatic(
        Task_FaultHandler,             // Task function
        "FaultHandler",                // Name of the task (for debugging)
        configMINIMAL_STACK_SIZE,   // Stack size in words
        NULL,                       // Task input parameter
        tskIDLE_PRIORITY + 3,       // Task priority
        FaultHandler_Task_Stack,       // Task handle
        &FaultHandler_Task_Buffer      // Static task buffer (optional)
    );

    vTaskStartScheduler();

    return 0;
}