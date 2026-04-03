#include "BPS_Tasks.h"
#include "config.h"
#include "CANbus.h"
#include "BPSCAN_can_msgs.h"

#define AMPERES_CAN_DELAY_MS 50
#define UNPACK_CURRENT(x)   ( (int32_t)(((uint32_t)(x)[2] << 24) | ((uint32_t)(x)[1] << 16) | ((uint32_t)(x)[0] << 8)) >> 8 )
#define UNPACK_RAWV(x)      ( (uint16_t)((x[4] << 8) | (uint16_t) x[3]) )

void Task_Amperes_Monitor() {
    bps_pack_current_t amperesData;

    while (1) {
    // Delays 100 ms
    vTaskDelay(AMPERES_MONITOR_TASK_DELAY_MS);

    // Receive from CAN
    uint8_t buffer[CAN_DLC_BPS_PACK_CURRENT] = {0};
    if (bps_can_recv(CAN_ID_BPS_PACK_CURRENT, buffer, CAN_DLC_BPS_PACK_CURRENT, AMPERES_CAN_DELAY_MS) != CAN_OK) {
        set_faultBit(BPS_CAN_ERROR);
    } else {
        amperesData.Main_Battery_Current =  UNPACK_CURRENT(buffer);
        amperesData.Main_Battery_Current_RawV =  UNPACK_RAWV(buffer);
    }

    // Set fault bits if needed
    if ((amperesData.Main_Battery_Current < OVERCURRENT_CHARGE_THRESHOLD_MA) ||
        (amperesData.Main_Battery_Current > OVERCURRENT_DISCHARGE_THRESHOLD_MA) ) {
        set_faultBit(BATTERY_OVERCURRENT_FAULT);
    }

    // Set event group bit
    xEventGroupSetBits(xWDogEventGroup_handle,      /* The event group being updated. */
                        AMPERES_MONITOR_DONE);      /* The bits being set. */
    }
}