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

// Shared RX/TX scratch buffer size. The module override/ack (0x69/0x669) are 8 bytes; the BPS
// command/ack (0x67/0x667) are 1 byte -- sized for the larger.
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

    // Charge OK = the pack can accept charge: max cell within the charge voltage AND temp limits
    // (override-relaxed where applicable), with the monitor-task hysteresis already applied. This is
    // purely a cell-limit readiness flag -- NOT the array/precharge/charging state (charge_is_enabled).
    bps_status_message->BPS_Charge_OK = ((get_state_bit(VOLT_OK_FOR_CHARGING) == STATE_BIT_SET) &&
                                         (get_state_bit(TEMP_OK_FOR_CHARGING) == STATE_BIT_SET))
                                            ? 1
                                            : 0;

    // Regen is opt-in: only allowed while the BPS_Regen_Allow command signal is active (drive-profile
    // master + regen bit + config gate), AND there is no fault, AND the pack's max cell voltage/temp
    // are below the regen setpoints (drive_profile_config.h). Reported to the VCU; the BPS does not
    // actuate regen. Overcurrent still faults regardless of this flag (see AmperesMonitorTask).
    bps_status_message->BPS_Regen_OK = (overrides_regen_allowed() &&
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

// Non-blocking poll of the BPS command + module override inputs (0x67/0x69) and broadcast of the
// current state (0x667/0x669). State only changes on a received message; acks are always sent so the
// sender sees we're alive and what we believe. The command ack reflects the EFFECTIVE state (after
// config gating). Also called by Task_Init during the startup window so commands are in effect early.
void process_overrides(void)
{
    uint8_t buf[OVERRIDE_PAYLOAD_DLC] = {0};

    if (car_can_recv(CAN_ID_BPS_COMMAND, buf, CAN_DLC_BPS_COMMAND, 0) == CAN_OK)
    {
        overrides_set_command(buf[0]);
    }

    if (car_can_recv(CAN_ID_BPS_MODULE_OVERRIDE, buf, CAN_DLC_BPS_MODULE_OVERRIDE, 0) == CAN_OK)
    {
        overrides_set_module_raw(buf);
    }

    overrides_pack_command_ack(buf);
    car_can_send(CAN_ID_BPS_COMMAND_ACK, buf, CAN_DLC_BPS_COMMAND_ACK, BPS_STATUS_CAN_DELAY_MS);

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

#if BPS_CMD_CONFIG_ADV_MPPT_CONTROL
// Config invariants the control law below depends on (compile-time, zero runtime cost). The most
// likely real-world breakage is someone editing the mV thresholds into an inconsistent order.
_Static_assert((MPPT_MAX_BOOST_MV / MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB) <= 32767,
               "MPPT boost limit exceeds int16 -- check MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB");
_Static_assert(MPPT_MIN_BOOST_MV < MPPT_MAX_BOOST_MV, "MPPT boost floor must be below the ceiling");
_Static_assert(MPPT_CHARGE_RESTART_MV <= MPPT_TAPER_START_MV, "restart hysteresis must sit at/below taper-start");
_Static_assert(MPPT_TAPER_START_MV <= MPPT_CV_TARGET_MV, "taper must start at/below the CV target");
// Drive-profile voltage ladder must stay strictly ordered so the master charge cutoff sits ABOVE the
// CV target (else it preempts the CV hold/taper) and BELOW the hard overvoltage fault (so the gentler
// open-array backstop fires before the emergency shutdown).
_Static_assert(MPPT_CV_TARGET_MV < OVERRIDE_CELL_CHARGING_VOLTAGE_THRESHOLD_MV,
               "CV target must stay below the master charge cutoff, else the cutoff preempts the CV hold");
_Static_assert(OVERRIDE_CELL_CHARGING_VOLTAGE_THRESHOLD_MV < OVERRIDE_CELL_OVERVOLTAGE_THRESHOLD_MV,
               "master charge cutoff must stay below the hard cell-overvoltage fault (the independent backstop)");
_Static_assert(MPPT_CV_DEADBAND_MV > 0 && MPPT_CV_DEADBAND_MV < (MPPT_CV_TARGET_MV - MPPT_TAPER_START_MV),
               "CV deadband must be positive and fit inside the taper band");
_Static_assert(MPPT_VLIMIT_STEP_MV > 0, "ceiling step must be positive");
_Static_assert(MPPT_CHARGE_TERMINATE_CURRENT_MA > 0, "termination current must be positive (|mA|)");

// Advanced MPPT control, used only while the drive override (0x67) is active. Closed loop on the
// pack's MAX CELL (not pack/bus voltage -- under imbalance the bus voltage tells you nothing about
// the fullest cell). Below MPPT_TAPER_START_MV the MPPTs do full power-point tracking; above it we
// hold the max cell near MPPT_CV_TARGET_MV by nudging the MPPT output-voltage ceiling each cycle
// (the TPEE tapers its own current as the bus nears the lowered ceiling). Charge ends on CURRENT --
// once the max cell has held in the CV band with the (net) charge current tapered below
// MPPT_CHARGE_TERMINATE_CURRENT_MA for MPPT_CHARGE_TERMINATE_TIME_MS, the MPPTs go off until the top
// cell relaxes/discharges below MPPT_CHARGE_RESTART_MV (hysteresis), then harvesting resumes.
// charge_is_enabled() stays the master gate; CELL_OVERVOLTAGE_FAULT (4250 mV) is the hard backstop.
static void mppt_drive_advanced(void)
{
    // Controller state, persisted across the 300 ms control cycles.
    static int32_t   s_vlimit_mV = MPPT_MAX_BOOST_MV;  // MPPT output-voltage ceiling (integrator state)
    static bool      s_terminated = false;             // charge-complete latch (cleared by restart hysteresis)
    static TickType_t s_taper_since = 0;               // when "in CV band + low current" began (0 = not yet)

    // Master gate: charging not allowed -> everything off, reset the controller so the next session
    // starts from the top of the ceiling.
    if (!charge_is_enabled())
    {
        disableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
        s_vlimit_mV = MPPT_MAX_BOOST_MV;
        s_terminated = false;
        s_taper_since = 0;
        return;
    }

    uint32_t max_cell_mV = get_max_cell_voltage();

    // Charge complete: hold the MPPTs off until the top cell relaxes/discharges below the restart
    // threshold, so we don't immediately re-start the instant the cell sags off the target.
    if (s_terminated)
    {
        if (max_cell_mV <= MPPT_CHARGE_RESTART_MV)
        {
            s_terminated = false;            // resume harvesting from the top of the ceiling
            s_vlimit_mV = MPPT_MAX_BOOST_MV;
            s_taper_since = 0;
        }
        else
        {
            disableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
            return;
        }
    }

    // Below the taper point there is no overcharge risk -- pull full power and arm the taper to
    // start from the top next time it is needed.
    if (max_cell_mV < MPPT_TAPER_START_MV)
    {
        s_vlimit_mV = MPPT_MAX_BOOST_MV;
        s_taper_since = 0;
        enableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
        return;
    }

    // CV taper: deadband-integral loop on the max cell. Cell over target -> lower the ceiling (less
    // current); cell below the hold band -> raise it (more current). Inside the band, hold. Clamp.
    if (max_cell_mV > MPPT_CV_TARGET_MV)
    {
        s_vlimit_mV -= MPPT_VLIMIT_STEP_MV;
    }
    else if (max_cell_mV < (MPPT_CV_TARGET_MV - MPPT_CV_DEADBAND_MV))
    {
        s_vlimit_mV += MPPT_VLIMIT_STEP_MV;
    }

    if (s_vlimit_mV > MPPT_MAX_BOOST_MV) s_vlimit_mV = MPPT_MAX_BOOST_MV;
    if (s_vlimit_mV < MPPT_MIN_BOOST_MV) s_vlimit_mV = MPPT_MIN_BOOST_MV;

    setAllMPPTsOutputVoltageLimit((int16_t)(s_vlimit_mV / MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB),
                                  BPS_STATUS_CAN_DELAY_MS);
    enableAllMPPTsConstantVoltage(BPS_STATUS_CAN_DELAY_MS);

    // Current-based termination: top cell held in the CV band (the deadband keeps it just under
    // target, so gate on the band's lower edge, not a strict == target) AND net charge current
    // tapered below the cutoff, continuously for the dwell time -> charge complete.
    int32_t pack_current_mA = get_pack_current(); // negative = charging
    bool charge_tapered = (max_cell_mV >= (MPPT_CV_TARGET_MV - MPPT_CV_DEADBAND_MV)) &&
                          (pack_current_mA > -MPPT_CHARGE_TERMINATE_CURRENT_MA);
    if (charge_tapered)
    {
        if (s_taper_since == 0)
        {
            s_taper_since = xTaskGetTickCount();
        }
        else if (Calculate_TimeDifference(xTaskGetTickCount(), s_taper_since) >=
                 pdMS_TO_TICKS(MPPT_CHARGE_TERMINATE_TIME_MS))
        {
            s_terminated = true;
            disableAllMPPTs(BPS_STATUS_CAN_DELAY_MS);
        }
    }
    else
    {
        s_taper_since = 0; // current rose again or cell dropped out of band -> restart the dwell timer
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
        // charge_is_enabled() (array precharged + pack within charge limits + no fault). When the
        // BPS_Adv_MPPT_Control command signal is active (drive-profile master + config + bit), run
        // the closed-loop max-cell CV taper + current-based termination in mppt_drive_advanced()
        // instead. Charge-disable and the fault handler also send an immediate boost-disable; this
        // just asserts the steady state.
#if BPS_CMD_CONFIG_ADV_MPPT_CONTROL
        if (overrides_adv_mppt_enabled())
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
