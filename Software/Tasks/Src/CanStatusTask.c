#include "common.h"
#include "BPS_Tasks.h"
#include "CarCAN_can_msgs.h"
#include "Contactors.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include <string.h>

// Converts the temp in mC to the centi-celcius used in the status
#define CONVERT_TEMP_FOR_STATUS(temp) ((temp) / 10)

#define BPS_STATUS_CAN_DELAY_MS 10u

// returns 0 if segemet is OK else it returns 1
static uint8_t get_segment_status(uint8_t segment_num)
{

    if (get_temp_segment_status(segment_num) == false)
    {
        return 1;
    }

    if (get_volt_segment_status(segment_num) == false)
    {
        return 1;
    }

    return 0;
}

static void pack_bps_status_message(const bps_status_t *status, uint8_t *can_data)
{
    // Byte 0: BPS_Fault [0:7]
    can_data[0] = status->BPS_Fault;

    // Byte 1: Contactor and OK States [8:13]
    // Bits 14 and 15 are unused, set to 0
    can_data[1] = 0x00;
    can_data[1] |= (status->BPS_Charge_OK & 0x01) << 0;                   // Bit 8
    can_data[1] |= (status->BPS_Regen_OK & 0x01) << 1;                    // Bit 9
    can_data[1] |= (status->HV_Plus_Contactor_State & 0x01) << 2;         // Bit 10
    can_data[1] |= (status->HV_Minus_Contactor_State & 0x01) << 3;        // Bit 11
    can_data[1] |= (status->Array_Contactor_State & 0x01) << 4;           // Bit 12
    can_data[1] |= (status->Array_Precharge_Contactor_State & 0x01) << 5; // Bit 13

    // Bytes 2, 3, 4: Main_Battery_Voltage [16:39] (24 bits)
    // Assuming little-endian packing
    can_data[2] = (uint8_t)(status->Main_Battery_Voltage & 0xFF);         // Bits 16-23
    can_data[3] = (uint8_t)((status->Main_Battery_Voltage >> 8) & 0xFF);  // Bits 24-31
    can_data[4] = (uint8_t)((status->Main_Battery_Voltage >> 16) & 0xFF); // Bits 32-39

    // Bytes 5, 6: Main_Battery_Avg_Temperature [40:55] (16 bits)
    // Cast to uint16_t to ensure safe bitwise shifting for signed integers
    uint16_t raw_temp = (uint16_t)status->Main_Battery_Avg_Temperature;
    can_data[5] = (uint8_t)(raw_temp & 0xFF);        // Bits 40-47
    can_data[6] = (uint8_t)((raw_temp >> 8) & 0xFF); // Bits 48-55

    can_data[7] = status->BPS_Segment0_Status;
    can_data[7] |= status->BPS_Segment1_Status << 1;
    can_data[7] |= status->BPS_Segment2_Status << 2;
    can_data[7] |= status->BPS_Segment3_Status << 3;
    can_data[7] |= status->BPS_Segment4_Status << 4;
    can_data[7] |= status->BPS_Segment5_Status << 5;
    can_data[7] |= status->BPS_Segment6_Status << 6;
    can_data[7] |= status->BPS_Segment7_Status << 7;
}

static void get_bps_status_information(bps_status_t *bps_status_message)
{

    if (system_has_faulted)
    {
        switch (first_fault_id)
        {
        case CELL_OVERVOLTAGE_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_OVERVOLTAGE;
            break;

        case CELL_UNDERVOLTAGE_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_UNDERVOLTAGE;
            break;

        case REGEN_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_REGEN;
            break;

        case CELL_OVERTEMP_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_OVERTEMPERATURE;
            break;

        case ELCON_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_ELCON;
            break;

        case PRECHARGE_TIMEOUT_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_ARRAY_PRECHARGE_TIMEOUT;
            break;

        case RTOS_WATCHDOG_ERROR:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_INTERNAL_WATCHDOG;
            break;

        case BQ_CHIP_FAULT:
        case VOLTTEMP_WATCHDOG_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_SEGMENT_WATCHDOG;
            break;

        case CONTACTOR_HV_PLUS_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_HV_PLUS_CONTACTOR_SENSE;
            break;

        case CONTACTOR_HV_MINUS_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_HV_MINUS_CONTACTOR_SENSE;
            break;

        case CONTACTOR_ARRAY_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_ARRAY_CONTACTOR_SENSE;
            break;

        case CONTACTOR_ARRAY_PRE_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_ARRAY_PCHG_CONTACTOR_SENSE;
            break;

        case BPS_ESTOP1_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_ESTOP_1;
            break;

        case BPS_ESTOP2_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_ESTOP_2;
            break;

        case BPS_ESTOP3_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_ESTOP_3;
            break;

        case PACK_OVERCURRENT_CHARGING_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_CHARGING_OVERCURRENT;
            break;

        case PACK_OVERCURRENT_DISCHARGING_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_DISCHARGING_OVERCURRENT;
            break;

        case AMPERES_WATCHDOG_FAULT:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_AMPERES_WATCHDOG;
            break;

        default:
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_INTERNAL_WATCHDOG;
            break;
        }
    }

    else
    {
        if (is_fault_set(NUM_FAULTS) == 0)
        {
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_OK;
        }
        else
        {
            bps_status_message->BPS_Fault = BPS_STATUS_BPS_FAULT_INTERNAL_WATCHDOG;
        }
    }

    bps_status_message->Main_Battery_Voltage = get_pack_voltage();
    bps_status_message->Main_Battery_Avg_Temperature = (CONVERT_TEMP_FOR_STATUS(get_avg_temp()));

    bps_status_message->BPS_Charge_OK = ((bps_status_message->BPS_Fault == BPS_STATUS_BPS_FAULT_OK) && (get_state_bit(VOLT_OK_FOR_CHARGING) == STATE_BIT_SET) && (get_state_bit(TEMP_OK_FOR_CHARGING) == STATE_BIT_SET) && get_state_bit(ELCON_OK_FOR_CHARGING) == STATE_BIT_SET) ? 1 : 0;

    bps_status_message->BPS_Regen_OK = (bps_status_message->BPS_Fault == BPS_STATUS_BPS_FAULT_OK) ? 1 : 0;

    bps_status_message->HV_Plus_Contactor_State = (contactor_get(HV_PLUS_CONTACTOR) == CONTACTOR_CLOSED) ? 1 : 0;
    bps_status_message->HV_Minus_Contactor_State = (contactor_get(HV_MINUS_CONTACTOR) == CONTACTOR_CLOSED) ? 1 : 0;
    bps_status_message->Array_Contactor_State = (contactor_get(ARRAY_CONTACTOR) == CONTACTOR_CLOSED) ? 1 : 0;
    bps_status_message->Array_Precharge_Contactor_State = (contactor_get(ARRAY_PRE_CONTACTOR) == CONTACTOR_CLOSED) ? 1 : 0;

    bps_status_message->BPS_Segment0_Status = get_segment_status(0);
    bps_status_message->BPS_Segment1_Status = get_segment_status(1);
    bps_status_message->BPS_Segment2_Status = get_segment_status(2);
    bps_status_message->BPS_Segment3_Status = get_segment_status(3);
    bps_status_message->BPS_Segment4_Status = get_segment_status(4);
    bps_status_message->BPS_Segment5_Status = get_segment_status(5);
    bps_status_message->BPS_Segment6_Status = get_segment_status(6);
    bps_status_message->BPS_Segment7_Status = get_segment_status(7);
}

void Task_Can_Status(void *pvParameters)
{

    TickType_t xLastWakeTime = xTaskGetTickCount();

    bps_status_t bps_status_message = {0};
    uint8_t bps_status_raw_can[CAN_DLC_BPS_STATUS] = {0};

    while (1)
    {

        toggleHeartbeat();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CAN_STATUS_TASK_DELAY_MS));

        get_bps_status_information(&bps_status_message);

        pack_bps_status_message(&bps_status_message, bps_status_raw_can);

        car_can_send(CAN_ID_BPS_STATUS, bps_status_raw_can, CAN_DLC_BPS_STATUS, BPS_STATUS_CAN_DELAY_MS);
    }
}
