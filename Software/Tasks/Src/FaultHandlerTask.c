#include "FaultHandlerTask.h"
#include "PrechargeTask.h" // for hprecharge_task handle
#include "EMC2305_Driver.h"

#define FAULT_LOOP_PRINTF_DELAY_MS 1000
#define FAULT_LOOP_PERIOD_MS 500

#define FAULT_PRINTF_COUNTER (FAULT_LOOP_PRINTF_DELAY_MS / FAULT_LOOP_PERIOD_MS)

void Fault_Loop(uint32_t fault_bit_index) {

    uint32_t fault_printf_debug_counter = 0;
    while (1)
    {
        fault_printf_debug_counter++;

        if (fault_printf_debug_counter >= FAULT_PRINTF_COUNTER)
        {
            handle_fault(fault_bit_index);
            fault_printf_debug_counter = 0;
        }

        toggleHeartbeat();
        vTaskDelay(pdMS_TO_TICKS(FAULT_LOOP_PERIOD_MS));
    }
}

void Task_FaultHandler(void* pvParameters) {

    while (true)
    {
        // Wait indefintiely for any fault bit to be set
        uint32_t fault_bit_index = faultBit_wait(NUM_FAULTS, portMAX_DELAY);



            LEDs_clear();
            LED_set(FAULT_LED, LED_ON);

            emergency_open_contactors();
            set_fans_MAX();

            handle_fault(fault_bit_index);

            Fault_Loop(fault_bit_index); // WILL NEVER RETURN - while(true)
        

    }
}
