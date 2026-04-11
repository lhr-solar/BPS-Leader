// todo

#include "BPS_Tasks.h"
#include "config.h"
#include "CANbus.h"
#include "BPSCAN_can_msgs.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"

// CAN timeout
#define AMPERES_CAN_TIMEOUT_MS AMPERES_MONITOR_TASK_DELAY_MS
#define AMPERES_CAN_TIMEOUT_FAULT_MS 500
#define AMPERES_CAN_TIMEOUT_TRIGGER_COUNT AMPERES_CAN_TIMEOUT_FAULT_MS / AMPERES_CAN_TIMEOUT_MS
#define CHARGING_THRESHOLD (-50)

// CAN message decoding
#define AMPERES_UNPACK_CURRENT_mA(x) ((int32_t)(((uint32_t)(x)[2] << 24) | ((uint32_t)(x)[1] << 16) | ((uint32_t)(x)[0] << 8)) >> 8)
#define AMPERES_UNPACK_RAW_mV(x) ((uint16_t)((x[4] << 8) | (uint16_t)x[3]))

// Printf period macros
#define AMPERES_LOOP_PRINTF_DELAY_MS 2000
#define AMPERES_PRINTF_COUNTER (AMPERES_LOOP_PRINTF_DELAY_MS / AMPERES_MONITOR_TASK_DELAY_MS)

// Global variable
bps_pack_current_t AmperesData = { 0 };

void Task_Amperes_Monitor() {
    uint8_t amperesCANFaultCount = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    uint32_t amps_printf_debug_counter = 0;

    while (1)
    {
        amps_printf_debug_counter++;

        // Delays 100 ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(AMPERES_MONITOR_TASK_DELAY_MS));

        // Receive from CAN
        uint8_t buffer[CAN_DLC_BPS_PACK_CURRENT] = { 0 };
        if (bps_can_recv(CAN_ID_BPS_PACK_CURRENT, buffer, CAN_DLC_BPS_PACK_CURRENT, AMPERES_CAN_TIMEOUT_MS) != CAN_OK)
        {
            amperesCANFaultCount++;
        }
        else
        {
            AmperesData.Main_Battery_Current = AMPERES_UNPACK_CURRENT_mA(buffer);
            AmperesData.Main_Battery_Current_RawV = AMPERES_UNPACK_RAW_mV(buffer);
            amperesCANFaultCount = 0;

            // Print current at lower rate
            if (amps_printf_debug_counter >= AMPERES_PRINTF_COUNTER)
            {
                printf("\r\n");
                printf("Pack Current: %li mA\r\n", AmperesData.Main_Battery_Current);
                printf("\r\n");
                amps_printf_debug_counter = 0;
            }
        }

        // Handle CAN fault
        if (amperesCANFaultCount >= AMPERES_CAN_TIMEOUT_TRIGGER_COUNT)
        {
            // printf("FAULT: AMPERES CAN RECV\r\n");
            set_faultBit(BPS_CAN_ERROR);
        }

        // Set fault bits if needed. If good, set the event group bit
        if (AmperesData.Main_Battery_Current < OVERCURRENT_CHARGE_THRESHOLD_mA) set_faultBit(PACK_OVERCURRENT_CHARGING_FAULT); 

        else if (AmperesData.Main_Battery_Current > OVERCURRENT_DISCHARGE_THRESHOLD_mA) set_faultBit(PACK_OVERCURRENT_DISCHARGING_FAULT);

        else
        {
            set_state_bit(AMPERES_MONITOR_GOOD, STATE_BIT_SET);
        }

        if (AmperesData.Main_Battery_Current > CHARGING_THRESHOLD)
        {
            set_state_bit(DISCHARGING_BATT_STATE, STATE_BIT_SET);
            set_state_bit(CHARGING_BATT_STATE, STATE_BIT_RESET);
            LED_set(CHARGING_LED, LED_OFF);
        }
        else
        {
            set_state_bit(DISCHARGING_BATT_STATE, STATE_BIT_RESET);
            set_state_bit(CHARGING_BATT_STATE, STATE_BIT_SET);
            LED_set(CHARGING_LED, LED_ON);
        }

        // Set event group bit so watchdog knows we ran
        xEventGroupSetBits(xWDogEventGroup_handle, AMPERES_MONITOR_DONE);
    }
}
