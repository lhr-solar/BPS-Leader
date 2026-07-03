//toDone

#include "BPS_Tasks.h"
#include "config.h"
#include "CANbus.h"
#include "BPSCAN_can_msgs.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"
#include "charge.h"
#include "overrides.h"

// CAN timeout. Equal to the task period (25 ms) by design: vTaskDelayUntil targets an ABSOLUTE wake
// time, so when the amperes board is silent the blocking recv simply consumes the period instead of
// stacking on top of it -- the loop still runs (and pets the watchdog via AMPERES_MONITOR_DONE) once
// per ~25 ms, it does not halve to 50 ms. When data is present recv returns immediately and the
// vTaskDelayUntil below provides the spacing. Either way the effective period stays one task delay.
#define AMPERES_CAN_TIMEOUT_MS AMPERES_MONITOR_TASK_DELAY_MS
#define AMPERES_WATCHDOG_TIMEOUT_MS 500

// CAN message decoding
#define AMPERES_UNPACK_CURRENT_mA(x) (((int32_t)(((uint32_t)(x)[3] << 24) | ((uint32_t)(x)[2] << 16) | ((uint32_t)(x)[1] << 8))) >> 8)
#define AMPERES_UNPACK_FAULT(x) ((uint8_t)((x[0])))

// Printf period macros
#define AMPERES_LOOP_PRINTF_DELAY_MS 2000
#define AMPERES_PRINTF_COUNTER (AMPERES_LOOP_PRINTF_DELAY_MS / AMPERES_MONITOR_TASK_DELAY_MS)

static TimerHandle_t amperes_watchdog_timer;
static StaticTimer_t amperes_timer_buffer;

static bool recv_amp_data = false;

// Global variable
bps_pack_current_t AmperesData = { 0 };

// Pack current published as a single aligned volatile word for cross-task readers (temperature,
// voltage, CAN-status, fault-handler tasks). Mirrors the g_avg_temp_mC / g_pack_voltage_mV pattern:
// a 32-bit aligned access is single-copy atomic on this MCU, so readers never see a torn value and
// no mutex is needed. Immune to tearing even if bps_pack_current_t is later regenerated packed.
static volatile int32_t g_pack_current_mA = 0;

int32_t get_pack_current(void) {
    return g_pack_current_mA;
}

static void vAmperesWatchdogCallback(TimerHandle_t amps_timer)
{

    if (recv_amp_data == false)
    {
        set_faultBit(AMPERES_WATCHDOG_FAULT);
    }
    else {
        recv_amp_data = false;
    }
}

void Task_Amperes_Monitor() {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    uint32_t amps_printf_debug_counter = 0;
    
    // variable used to keep track if the car is currently charging
    bool is_charging = false;

    // Make timer for watchdog
    amperes_watchdog_timer = xTimerCreateStatic(
        "Amperes Watchdog",                         /* Name of the timer */
        pdMS_TO_TICKS(AMPERES_WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                                  /* auto-reload */
        (void *)0,                               /* Timer ID */
        vAmperesWatchdogCallback,            /* Callback function */
        &amperes_timer_buffer                       /* Buffer to hold timer data */
    );

    // Do NOT start the watchdog here: arming it before the first message would false-trip
    // AMPERES_WATCHDOG_FAULT while the amperes board is still booting (finding 1). It is armed on
    // first contact below. Startup HV-close is separately gated on AMPERES_MONITOR_GOOD (which now
    // also requires fresh data), so a board that never boots still cannot close contactors.

    while (1)
    {
        amps_printf_debug_counter++;

        // whether a fresh amperes CAN message was decoded this cycle (startup coverage gate)
        bool fresh_amp_data = false;

        // Delays 100 ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(AMPERES_MONITOR_TASK_DELAY_MS));

        // Receive from CAN. Block once for a frame, then drain any backlog non-blocking so we act on
        // the FRESHEST current: bps_can_recv pulls oldest-first from a depth-5 circular queue, so a
        // single read would hand us stale current on a backlog -- bad for overcurrent detection.
        uint8_t buffer[CAN_DLC_BPS_PACK_CURRENT] = { 0 };
        bool got_amp_frame = (bps_can_recv(CAN_ID_BPS_PACK_CURRENT, buffer, CAN_DLC_BPS_PACK_CURRENT, AMPERES_CAN_TIMEOUT_MS) == CAN_OK);
        while (bps_can_recv(CAN_ID_BPS_PACK_CURRENT, buffer, CAN_DLC_BPS_PACK_CURRENT, 0) == CAN_OK)
        {
            got_amp_frame = true; // keep only the most recent frame (buffer overwritten each read)
        }
        if (got_amp_frame)
        {
            recv_amp_data = true;
            fresh_amp_data = true;

            // Arm the watchdog on first contact (auto-reload thereafter).
            if (xTimerIsTimerActive(amperes_watchdog_timer) == pdFALSE)
            {
                xTimerStart(amperes_watchdog_timer, 0);
            }

            AmperesData.Main_Battery_Current = AMPERES_UNPACK_CURRENT_mA(buffer);
            g_pack_current_mA = AmperesData.Main_Battery_Current; // publish latest for cross-task readers
            AmperesData.BPS_Amperes_Fault = AMPERES_UNPACK_FAULT(buffer);

            // Print current at lower rate
            if (amps_printf_debug_counter >= AMPERES_PRINTF_COUNTER)
            {
                printf("\r\n");
                printf("Pack Current: %li mA\r\n", AmperesData.Main_Battery_Current);
                printf("\r\n");
                amps_printf_debug_counter = 0;
            }
        }

        // Set fault bits if needed. If good, set the event group bit.
        // Overcurrent is purely current-threshold based and must stay independent of the
        // charge/regen-OK states: even if charge or regen is "not OK", excess current still faults.
        if (AmperesData.Main_Battery_Current < overrides_overcurrent_charge_mA()) set_faultBit(PACK_OVERCURRENT_CHARGING_FAULT); 

        else if (AmperesData.Main_Battery_Current > overrides_overcurrent_discharge_mA()) set_faultBit(PACK_OVERCURRENT_DISCHARGING_FAULT);

        else if (AmperesData.BPS_Amperes_Fault != BPS_PACK_CURRENT_BPS_AMPERES_FAULT_OK) {
            switch (AmperesData.BPS_Amperes_Fault) {
                case BPS_PACK_CURRENT_BPS_AMPERES_FAULT_OUT_OF_BOUNDS:
                    set_faultBit(AMPERES_WATCHDOG_FAULT);
                    break;
                case BPS_PACK_CURRENT_BPS_AMPERES_FAULT_OVER_CURRENT_DISCHARGE_:
                    set_faultBit(PACK_OVERCURRENT_DISCHARGING_FAULT);
                    break;
                case BPS_PACK_CURRENT_BPS_AMPERES_FAULT_OVER_CURRENT_CHARGE_:
                    set_faultBit(PACK_OVERCURRENT_CHARGING_FAULT);
                    break;
                case BPS_PACK_CURRENT_BPS_AMPERES_FAULT_MESSAGE_WATCHDOG:
                    set_faultBit(AMPERES_WATCHDOG_FAULT);
                    break;
            }
        }
        else
        {
            // Only declare the monitor "good" on FRESH data, so the startup HV-close gate can't be
            // satisfied by stale/all-zero defaults before the amperes board has actually reported.
            if (fresh_amp_data && get_state_bit(AMPERES_MONITOR_GOOD) != STATE_BIT_SET) {
                set_state_bit(AMPERES_MONITOR_GOOD, STATE_BIT_SET);
            }
        }

        if (AmperesData.Main_Battery_Current > CHARGING_THRESHOLD_MA)  
        {
            if (is_charging) {
                set_state_bit(DISCHARGING_BATT_STATE, STATE_BIT_SET);
                set_state_bit(CHARGING_BATT_STATE, STATE_BIT_RESET);
                LED_set(CHARGING_LED, LED_OFF);
                is_charging = false;
            }
        }
        else
        {
            if (!is_charging) {
                set_state_bit(DISCHARGING_BATT_STATE, STATE_BIT_RESET);
                set_state_bit(CHARGING_BATT_STATE, STATE_BIT_SET);
                LED_set(CHARGING_LED, LED_ON);
                is_charging = true;
            }
        }

        // Set event group bit so watchdog knows we ran
        xEventGroupSetBits(xWDogEventGroup_handle, AMPERES_MONITOR_DONE);
    }
}
