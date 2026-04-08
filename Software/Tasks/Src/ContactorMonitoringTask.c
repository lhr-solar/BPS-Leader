#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "Contactors.h"

void Task_Contactor_Monitor(void *pvParameters)
{   

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {   
        bool good_state = true;

        // Delays CONTACTOR_MONITOR_TASK_DELAY_MS
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTACTOR_MONITOR_TASK_DELAY_MS));

        // confirm every contactor is in the correct state
        if (contactor_verify(HV_PLUS_CONTACTOR) != CONTACTOR_OK)
        {   
            estop_status_t estop_status = contactor_estop_checker();

            if (estop_status == ESTOP_OK)
            {
                set_faultBit(CONTACTOR_HV_PLUS_FAULT);
            }
            else if (estop_status == ESTOP1_FAULT) set_faultBit(BPS_ESTOP1_FAULT);
            else if (estop_status == ESTOP2_FAULT) set_faultBit(BPS_ESTOP2_FAULT);
            else set_faultBit(BPS_ESTOP3_FAULT);

            good_state = false;
        }

        if (contactor_verify(HV_MINUS_CONTACTOR) != CONTACTOR_OK)
        {
            set_faultBit(CONTACTOR_HV_MINUS_FAULT);
            good_state = false;
        }

        if (contactor_verify(ARRAY_CONTACTOR) != CONTACTOR_OK)
        {
            set_faultBit(CONTACTOR_ARRAY_FAULT);
            good_state = false;
        }

        if (contactor_verify(ARRAY_PRE_CONTACTOR) != CONTACTOR_OK)
        {
            set_faultBit(CONTACTOR_ARRAY_PRE_FAULT);
            good_state = false;
        }

        if (good_state) {
            if (get_task_bit(CONTACTOR_MONITOR) == 0) {
                set_task_bit(CONTACTOR_MONITOR);
            }
        }
    }
}