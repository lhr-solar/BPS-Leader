#include "FaultHandlerTask.h"
#include "PrechargeTask.h" // for hprecharge_task handle
#include "EMC2305_Driver.h"
#include "CANbus.h"

#define FAULT_LOOP_PRINTF_DELAY_MS 1000
#define FAULT_LOOP_PERIOD_MS 500

#define FAULT_PRINTF_COUNTER (FAULT_LOOP_PRINTF_DELAY_MS / FAULT_LOOP_PERIOD_MS)

#define FAULT_VAL_CAN_DELAY_MS 5u

// Snapshot of every module's voltage/temperature, captured at the instant the
// system faults. Broadcast on 0xF (BPS_Fault_Val_Arr) only while faulted.
static bps_fault_val_arr_t fault_val_snapshot[NUM_BATTERY_MODULES];

static void capture_fault_val_snapshot(void)
{
    for (uint8_t i = 0; i < NUM_BATTERY_MODULES; i++)
    {
        fault_val_snapshot[i].BPS_Tap_idx = i;
        fault_val_snapshot[i].BPS_Voltage_Tap_Data = (uint16_t)get_module_voltage(i);
        fault_val_snapshot[i].BPS_Temperature_Tap_Data = (int32_t)get_module_temperature(i);
    }
}

// Send the captured snapshot, cycling through every module idx (like the other arrays)
static void send_fault_val_snapshot(void)
{
    for (uint8_t i = 0; i < NUM_BATTERY_MODULES; i++)
    {
        uint8_t buf[CAN_DLC_BPS_FAULT_VAL_ARR] = {0};

        // bits[0:4] tap idx
        buf[0] = fault_val_snapshot[i].BPS_Tap_idx & 0x1F;
        // bits[8:23] voltage (mV)
        buf[1] = (uint8_t)(fault_val_snapshot[i].BPS_Voltage_Tap_Data & 0xFF);
        buf[2] = (uint8_t)((fault_val_snapshot[i].BPS_Voltage_Tap_Data >> 8) & 0xFF);
        // bits[24:47] temperature (mC, 24-bit signed)
        buf[3] = (uint8_t)(fault_val_snapshot[i].BPS_Temperature_Tap_Data & 0xFF);
        buf[4] = (uint8_t)((fault_val_snapshot[i].BPS_Temperature_Tap_Data >> 8) & 0xFF);
        buf[5] = (uint8_t)((fault_val_snapshot[i].BPS_Temperature_Tap_Data >> 16) & 0xFF);

        car_can_send(CAN_ID_BPS_FAULT_VAL_ARR, buf, CAN_DLC_BPS_FAULT_VAL_ARR, FAULT_VAL_CAN_DELAY_MS);
    }
}

void Fault_Loop(uint32_t fault_bit_index)
{

    uint32_t fault_printf_debug_counter = 0;
    while (1)
    {
        fault_printf_debug_counter++;

        if (fault_printf_debug_counter >= FAULT_PRINTF_COUNTER)
        {
            handle_fault(fault_bit_index);
            fault_printf_debug_counter = 0;
        }

        // Broadcast the battery state captured at fault (only sent while faulted)
        send_fault_val_snapshot();

        toggleHeartbeat();
        vTaskDelay(pdMS_TO_TICKS(FAULT_LOOP_PERIOD_MS));
    }
}

void Task_FaultHandler(void *pvParameters)
{

    while (true)
    {
        // Wait indefintiely for any fault bit to be set
        uint32_t fault_bit_index = faultBit_wait(NUM_FAULTS, portMAX_DELAY);

        // Capture the battery state right when the fault occurs (for 0xF broadcast)
        capture_fault_val_snapshot();

        LEDs_clear();
        LED_set(FAULT_LED, LED_ON);
        LED_setStrobe(LED_ON);

        emergency_open_contactors();
        set_fans_MAX();

        handle_fault(fault_bit_index);

        Fault_Loop(fault_bit_index); // WILL NEVER RETURN - while(true)
    }
}
