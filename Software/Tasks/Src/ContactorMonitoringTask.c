#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "Contactors.h"

void Task_Contactor_Monitor()
{

    while (1)
    {

        // Delays CONTACTOR_MONITOR_TASK_DELAY_MS
        vTaskDelay(CONTACTOR_MONITOR_TASK_DELAY_MS);

        // confirm every contactor is in the correct state
        if (contactor_get_command_state(HV_PLUS_CONTACTOR) != contactor_get(HV_PLUS_CONTACTOR))
        {
            if (contactor_estop_checker() == ESTOP_OK)
            {
                set_faultBit(CONTACTOR_UNEXPECTED_STATE_FAULT);
            }
            else
            {
                set_faultBit(ESTOP_FAULT);
            }
        }

        if (contactor_get_command_state(HV_MINUS_CONTACTOR) != contactor_get(HV_MINUS_CONTACTOR))
        {
            set_faultBit(CONTACTOR_UNEXPECTED_STATE_FAULT);
        }

        if (contactor_get_command_state(ARRAY_CONTACTOR) != contactor_get(ARRAY_CONTACTOR))
        {
            set_faultBit(CONTACTOR_UNEXPECTED_STATE_FAULT);
        }

        if (contactor_get_command_state(ARRAY_PRE_CONTACTOR) != contactor_get(ARRAY_PRE_CONTACTOR))
        {
            set_faultBit(CONTACTOR_UNEXPECTED_STATE_FAULT);
        }

        // Set event group bit
        // xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
        //                    TEMP_MONITOR_DONE);         /* The bits being set. */
    }
}