#include "BPS_Tasks.h"
#include "stm32xx_hal.h"

void Task_Voltage_Monitor(){

    while(1){
        // Delays 5 ms
        vTaskDelay(5);
        
        // Set event group bit
        xEventGroupSetBits(xEventGroupHandle,   /* The event group being updated. */
                           TASK2_BIT);          /* The bits being set. */

        // Toggle Pin for debug
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
    }
    
}