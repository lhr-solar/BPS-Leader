#include "FreeRTOS.h" /* Must come first. */
#include "task.h" 
#include "BPS_Tasks.h"
#include "pinConfig.h"
#include "stm32xx_hal.h"

// Task Stack Arrays
StackType_t Task_Init_Stack_Array[ TASK_INIT_STACK_SIZE ];
StackType_t Task_Temperature_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
StackType_t Task_Voltage_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
StackType_t Task_Amperes_Stack_Array[ TASK_AMPERES_MONITOR_STACK_SIZE ];
StackType_t Task_Petwdog_Stack_Array[ TASK_PETWDOG_STACK_SIZE ];

// Task Buffers
StaticTask_t Task_Init_Buffer;
StaticTask_t Task_Temperature_Buffer;
StaticTask_t Task_Voltage_Buffer;
StaticTask_t Task_Amperes_Buffer;
StaticTask_t Task_Petwdog_Buffer;



int main() {
    HAL_Init();

    xTaskCreateStatic(
                    Task_Init, /* The function that implements the task. */
                    "Init Task", /* Text name for the task. */
                    configMINIMAL_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
                    (void*)NULL, /* Paramter passed into the task. */
                    TASK_INIT_PRIO, /* Task Prioriy. */
                    Task_Init_Stack_Array, /* Stack array. */
                    &Task_Init_Buffer  /* Buffer for static allocation. */
   );

       GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = HEARTBEATPIN
    };
    
    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable clock for GPIOA
    HAL_GPIO_Init(GPIOA, &led_config); // initialize GPIOA with led_config

    while(1){
        HAL_GPIO_TogglePin(HEARTBEATPORT, HEARTBEATPIN);
        HAL_Delay(100);
    }

    // Start the scheduler
    vTaskStartScheduler();

    while(1){
        // Scheduler should've started by now
        // Code should never enter this point
    }
    
    return 0;
}
