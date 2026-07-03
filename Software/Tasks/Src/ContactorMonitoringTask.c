#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "Contactors.h"

void Task_Contactor_Monitor(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // 1. Initialize history variables BEFORE the loop starts
    // (Assuming  contactor_get returns an enum or bool of the physical state)
    contactor_state_t prev_hv_plus = contactor_get(HV_PLUS_CONTACTOR);
    contactor_state_t prev_hv_minus = contactor_get(HV_MINUS_CONTACTOR);
    contactor_state_t prev_array = contactor_get(ARRAY_CONTACTOR);
    contactor_state_t prev_array_pre = contactor_get(ARRAY_PRE_CONTACTOR);

    while (1)
    {
        bool all_contactors_good = true;

        // Delays CONTACTOR_MONITOR_TASK_DELAY_MS
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTACTOR_MONITOR_TASK_DELAY_MS));

        // ==========================================
        // STATE CHANGE DETECTION
        // ==========================================

        // 2. Read the current physical states
        contactor_state_t curr_hv_plus = contactor_get(HV_PLUS_CONTACTOR);
        contactor_state_t curr_hv_minus = contactor_get(HV_MINUS_CONTACTOR);
        contactor_state_t curr_array = contactor_get(ARRAY_CONTACTOR);
        contactor_state_t curr_array_pre = contactor_get(ARRAY_PRE_CONTACTOR);

        // 3. Compare and handle transitions
        if (curr_hv_plus != prev_hv_plus)
        {
            printf("================================\r\n");
            if (curr_hv_plus == CONTACTOR_CLOSED) {
                printf("HV PLUS CONTACTOR CLOSED\r\n");
            } else {
                printf("HV PLUS CONTACTOR OPENED\r\n");
            }
            printf("================================\r\n");
            
            prev_hv_plus = curr_hv_plus; // Update history for the next cycle
        }

        if (curr_hv_minus != prev_hv_minus)
        {
            printf("================================\r\n");
            if (curr_hv_minus == CONTACTOR_CLOSED) {
                printf("HV MINUS CONTACTOR CLOSED\r\n");
            } else {
                printf("HV MINUS CONTACTOR OPENED\r\n");
            }
            printf("================================\r\n");
            
            prev_hv_minus = curr_hv_minus;
        }

        if (curr_array != prev_array)
        {
            printf("================================\r\n");
            if (curr_array == CONTACTOR_CLOSED) {
                printf("ARRAY CONTACTOR CLOSED\r\n");
            } else {
                printf("ARRAY CONTACTOR OPENED\r\n");
            }
            printf("================================\r\n");
            
            prev_array = curr_array;
        }

        if (curr_array_pre != prev_array_pre)
        {
            printf("================================\r\n");
            if (curr_array_pre == CONTACTOR_CLOSED) {
                printf("ARRAY PRECHARGE CONTACTOR CLOSED\r\n");
            } else {
                printf("ARRAY PRECHARGE CONTACTOR OPENED\r\n");
            }
            printf("================================\r\n");
            
            prev_array_pre = curr_array_pre;
        }

        // ==========================================
        // FAULT AND VERIFICATION LOGIC
        // ==========================================

        estop_status_t estop_status = contactor_estop_checker();

        if (estop_status != ESTOP_OK)
            all_contactors_good = false;

        if (estop_status == ESTOP1_FAULT)
            set_faultBit(BPS_ESTOP1_FAULT);
        else if (estop_status == ESTOP2_FAULT)
            set_faultBit(BPS_ESTOP2_FAULT);
        else if (estop_status == ESTOP3_FAULT)
            set_faultBit(BPS_ESTOP3_FAULT);

        // Confirm every contactor is in the correct state (expected command state matches physical pin sense readings OR callback timer is still running)
        if (contactor_verify(HV_PLUS_CONTACTOR) != CONTACTOR_OK)
        {
            set_faultBit(CONTACTOR_HV_PLUS_FAULT);
            all_contactors_good = false;
        }

        if (contactor_verify(HV_MINUS_CONTACTOR) != CONTACTOR_OK)
        {
            set_faultBit(CONTACTOR_HV_MINUS_FAULT);
            all_contactors_good = false;
        }

        if (contactor_verify(ARRAY_CONTACTOR) != CONTACTOR_OK)
        {
            set_faultBit(CONTACTOR_ARRAY_FAULT);
            all_contactors_good = false;
        }

        if (contactor_verify(ARRAY_PRE_CONTACTOR) != CONTACTOR_OK)
        {
            set_faultBit(CONTACTOR_ARRAY_PRE_FAULT);
            all_contactors_good = false;
        }

        if (all_contactors_good)
        {
            set_state_bit(CONTACTOR_MONITOR_GOOD, STATE_BIT_SET);
        }

        // Check in with the RTOS watchdog (one of the ALL_TASKS_DONE bits).
        xEventGroupSetBits(xWDogEventGroup_handle, CONTACTOR_MONITOR_DONE);
    }
}