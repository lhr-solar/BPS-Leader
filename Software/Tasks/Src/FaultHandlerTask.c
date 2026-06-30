#include "FaultHandlerTask.h"
#include "PrechargeTask.h" // for hprecharge_task handle
#include "EMC2305_Driver.h"
#include "CANbus.h"
#include "overrides.h"
#include "charge.h"
#include "TPEE_Utils.h"

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

// On fault: always broadcast the fault immediately + set indication/cooling, then either
// open every contactor at once (hard) or perform the sequenced soft shutdown. The soft
// path broadcasts first so the VCU can command zero torque and open the motor contactors
// before we open the main battery contactors, so bus current has fallen and we avoid
// inductive overvoltage / contactor arcing. Every hard fault funnels through here, so the
// configured EMERGENCY_SOFT_SHUTDOWN_MODE governs all emergency shutdowns.
static void fault_shutdown_sequence(void)
{
    // Tell the rest of the car immediately (VCU acts on the BPS_Status fault field; this
    // send is the t=0 reference for the VCU's motor shutdown timeline).
    send_bps_status_now();

    // Fault indication + max cooling (both shutdown modes).
    LEDs_clear();
    LED_set(FAULT_LED, LED_ON);
    LED_setStrobe(LED_ON);
    set_fans_MAX();

    if (!shutdown_soft_active(EMERGENCY_SOFT_SHUTDOWN_MODE))
    {
        // Hard shutdown: boost disable + open every contactor immediately.
        disableAllMPPTs(FAULT_SHUTDOWN_INTERCONTACTOR_MS);
        emergency_open_contactors();
        return;
    }

    // 1) Soft-drop the solar array: boost disable -> MPPT wind-down -> open array + pchg.
    array_shutdown(EMERGENCY, true);

    // 2) Wait out the rest of the window so HV+ opens ~FAULT_SHUTDOWN_HV_DELAY_MS after the
    //    status TX (motor side has zeroed torque + opened its contactors, bus current fallen).
    vTaskDelay(pdMS_TO_TICKS(FAULT_SHUTDOWN_HV_DELAY_MS - FAULT_SHUTDOWN_MPPT_DELAY_MS - FAULT_SHUTDOWN_INTERCONTACTOR_MS));

    // 3) Open the main battery contactors: high side (HV+) first, low side (HV-) last.
    contactor_set(HV_PLUS_CONTACTOR, CONTACTOR_OPEN, 0, EMERGENCY);
    vTaskDelay(pdMS_TO_TICKS(FAULT_SHUTDOWN_INTERCONTACTOR_MS));
    contactor_set(HV_MINUS_CONTACTOR, CONTACTOR_OPEN, 0, EMERGENCY);
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

        // Sequenced soft shutdown: broadcast fault -> drop array -> wait for motor side
        // to zero torque / open its contactors -> open HV+ then HV-.
        fault_shutdown_sequence();

        handle_fault(fault_bit_index);

        Fault_Loop(fault_bit_index); // WILL NEVER RETURN - while(true)
    }
}
