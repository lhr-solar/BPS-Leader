#include "FaultHandlerTask.h"
#include "PrechargeTask.h" // for hprecharge_task handle

#define FAULT_LOOP_PRINTF_DELAY_MS 1000
#define FAULT_LOOP_PERIOD_MS 500

#define FAULT_PRINTF_COUNTER (FAULT_LOOP_PRINTF_DELAY_MS / FAULT_LOOP_PERIOD_MS)

EventBits_t fault_bits = 0;

void Kill_Precharge_Task() {
    if (hprecharge_task != NULL)
    {
        vTaskDelete(hprecharge_task);
    }
}

// Print faults & set relevant LEDs
static void print_fault() {


    printf("================================\r\n");

    if (fault_bits == 0) {
        printf("Fault Handler Broken!\r\n");
    }
    else {
        printf("FAULT: %s\r\n", fault_bit_strings[__builtin_ctz(fault_bits)]);
    } 

    printf("================================\r\n");
}

void Fault_Loop() {

    uint32_t fault_printf_debug_counter = 0;
    while (1)
    {
        fault_printf_debug_counter++;

        if (fault_printf_debug_counter >= FAULT_PRINTF_COUNTER)
        {
            print_fault();
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
        fault_bits = faultBit_wait(NUM_FAULTS, portMAX_DELAY);

        if (fault_bits != 0)
        {

            LEDs_clear();
            LED_set(FAULT_LED, LED_ON);

            Kill_Precharge_Task();
            emergency_open_contactors();

            print_fault();

            Fault_Loop(); // WILL NEVER RETURN - while(true)
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
