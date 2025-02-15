#include "BPS_Tasks.h"
#include "stm32xx_hal.h"

void Task_Temperature_Monitor(){

    while(1){
        // Delays 10 ms
        vTaskDelay(10);

        // Set event group bit
        xEventGroupSetBits(xEventGroupHandle,   /* The event group being updated. */
                           TASK1_BIT);          /* The bits being set. */

        // Toggle Pin for debug
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
    }
    
}