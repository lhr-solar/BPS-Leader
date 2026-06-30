#include "common.h"
#include "BPS_Tasks.h"
#include "CarCAN_can_msgs.h"
#include "Contactors.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "overrides.h"
#include "charge.h"
#include "TPEE_Utils.h"
#include <string.h>

// Converts the temp in mC to the centi-celcius used in the status
#define CONVERT_TEMP_FOR_STATUS(temp) ((temp) / 10)

#define BPS_STATUS_CAN_DELAY_MS 10u

// All override / override-ack messages (0x67/0x69/0x667/0x669) are 8 bytes
#define OVERRIDE_PAYLOAD_DLC 8u

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

    bps_status_message->BPS_Charge_OK = charge_is_enabled() ? 1 : 0;

    // Regen is opt-in: only allowed while the drive override (0x67) is active, AND there is no
    // fault, AND the pack's max cell voltage/temp are below the regen setpoints
    // (drive_profile_config.h). Reported to the VCU; the BPS does not actuate regen. Overcurrent
    // still faults regardless of this flag (see AmperesMonitorTask).
    bps_status_message->BPS_Regen_OK = ((overrides_get_drive() != 0) &&
                                        (bps_status_message->BPS_Fault == BPS_STATUS_BPS_FAULT_OK) &&
                                        (get_state_bit(VOLT_OK_FOR_REGEN) == STATE_BIT_SET) &&
                                        (get_state_bit(TEMP_OK_FOR_REGEN) == STATE_BIT_SET))
                                           ? 1
                                           : 0;

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

// Non-blocking poll of the override inputs (0x67/0x69) and broadcast of the
// current override state (0x667/0x669). State only changes on a received message;
// acks are always sent so the sender sees we're alive and what we believe.
// Also called by Task_Init during the startup window so overrides are in effect early.
void process_overrides(void)
{
    uint8_t buf[OVERRIDE_PAYLOAD_DLC] = {0};

    if (car_can_recv(CAN_ID_BPS_OVERRIDE, buf, CAN_DLC_BPS_OVERRIDE, 0) == CAN_OK)
    {
        overrides_set_drive(buf[0] & 0x1u);
    }

    if (car_can_recv(CAN_ID_BPS_MODULE_OVERRIDE, buf, CAN_DLC_BPS_MODULE_OVERRIDE, 0) == CAN_OK)
    {
        overrides_set_module_raw(buf);
    }

    overrides_pack_drive_ack(buf);
    car_can_send(CAN_ID_BPS_OVERRIDE_ACK, buf, CAN_DLC_BPS_OVERRIDE_ACK, BPS_STATUS_CAN_DELAY_MS);

    overrides_pack_module_ack(buf);
    car_can_send(CAN_ID_BPS_MODULE_OVERRIDE_ACK, buf, CAN_DLC_BPS_MODULE_OVERRIDE_ACK, BPS_STATUS_CAN_DELAY_MS);
}

void send_bps_status_now(void)
{
    bps_status_t bps_status_message = {0};
    uint8_t bps_status_raw_can[CAN_DLC_BPS_STATUS] = {0};

    get_bps_status_information(&bps_status_message);
    pack_bps_status_message(&bps_status_message, bps_status_raw_can);
    car_can_send(CAN_ID_BPS_STATUS, bps_status_raw_can, CAN_DLC_BPS_STATUS, BPS_STATUS_CAN_DELAY_MS);
}

#if ADVANCED_MPPT_CONTROL
_Static_assert((MPPT_MAX_BOOST_MV / MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB) <= 32767,
               "MPPT boost limit exceeds int16 -- check MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB");

// Advanced 3-region MPPT control, used only while the drive override (0x67) is active. The MPPT's
// own constant-voltage mode does the current taper in region B (it reduces current as the bus
// reaches the ceiling), so we just pick the mode by max-cell region. charge_is_enabled()
// (precharged + within the (override-relaxed) charge limits + no fault) stays the gate, and the
// BPS hard cell-overvoltage fault remains the independent safety backstop.
static void mppt_drive_advanced(void)
{
    if (!charge_is_enabled())
    {
        disableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
        return;
    }

    uint32_t max_cell_mV = get_max_cell_voltage();

    if (max_cell_mV >= MPPT_INHIBIT_MV)
    {
        // Region C: stop charging entirely
        disableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
    }
    else if (max_cell_mV >= MPPT_TAPER_START_MV)
    {
        // Region B: hold the bus at the boost ceiling; the MPPT tapers current itself
        setAllMPPTsOutputVoltageLimit((int16_t)(MPPT_MAX_BOOST_MV / MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB),
                                      BPS_STATUS_CAN_DELAY_MS);
        enableAllMPPTsConstantVoltage(BPS_STATUS_CAN_DELAY_MS);
    }
    else
    {
        // Region A: full MPPT power tracking
        enableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
    }
}
#endif

void Task_Can_Status(void *pvParameters)
{

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {

        toggleHeartbeat();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CAN_STATUS_TASK_DELAY_MS));

        send_bps_status_now();

        process_overrides();

        // Drive the MPPT boost at the status cadence. Default: plain enable/disable tracking
        // charge_is_enabled() (array precharged + pack within charge limits + no fault). With
        // ADVANCED_MPPT_CONTROL enabled AND the drive override active, use the 3-region
        // (full MPPT / CV taper / off) strategy instead. Charge-disable and the fault handler
        // also send an immediate boost-disable; this just asserts the steady state.
#if ADVANCED_MPPT_CONTROL
        if (overrides_get_drive() != 0)
        {
            mppt_drive_advanced();
        }
        else
#endif
        {
            if (charge_is_enabled())
            {
                enableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
            }
            else
            {
                disableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
            }
        }
    }
}
