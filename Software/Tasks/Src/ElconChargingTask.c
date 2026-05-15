#include "BPS_Tasks.h"
#include "config.h"
#include "CANbus.h"
#include "CarCAN_can_msgs.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"
#include "Contactors.h"
#include "faultHandler.h"

// Charger Interface Status bit positions (little-endian, 5-byte message)
// Bit 32 = byte[4] bit 0: Comm_ok  (1 = yes, 0 = no)
// Bit 33 = byte[4] bit 1: Fault present (1 = yes, 0 = no)
#define CHARGER_COMM_OK_MASK (1u << 0)
#define CHARGER_FAULT_PRESENT_MASK (1u << 1)

void Task_Elcon_Charging()
{
    // Default to not OK until charger confirms otherwise
    set_state_bit(ELCON_OK_FOR_CHARGING, STATE_BIT_RESET);
    printf("[ELCON] Task started, waiting for charger connection...\n");

    while (1)
    {
        uint8_t charger_buf[CAN_DLC_CHARGERINTERFACE_STATUS] = {0};

        // Block indefinitely, no point running until charger is  connected
        if (car_can_recv(CAN_ID_CHARGERINTERFACE_STATUS, charger_buf,
                         CAN_DLC_CHARGERINTERFACE_STATUS, portMAX_DELAY) != CAN_OK)
        {
            printf("[ELCON] car_can_recv error, retrying...\n");
            continue;
        }

        bool comm_ok      = (charger_buf[4] & CHARGER_COMM_OK_MASK) != 0;
        bool fault_present = (charger_buf[4] & CHARGER_FAULT_PRESENT_MASK) != 0;

        printf("[ELCON] Message received: comm_ok=%d, fault_present=%d\n",
               comm_ok, fault_present);

        if (comm_ok && !fault_present)
        {
            printf("[ELCON] Charger OK, setting ELCON_OK_FOR_CHARGING.\n");
            set_state_bit(ELCON_OK_FOR_CHARGING, STATE_BIT_SET);
        }
        else
        {
            printf("[ELCON] Charger NOT OK (comm_ok=%d, fault_present=%d), clearing ELCON_OK_FOR_CHARGING.\n",
                   comm_ok, fault_present);
            set_state_bit(ELCON_OK_FOR_CHARGING, STATE_BIT_RESET);

            if (fault_present)
            {
                printf("[ELCON] Fault reported by charger, setting ELCON_FAULT.\n");
                set_faultBit(ELCON_FAULT);
            }
        }
    }
}
