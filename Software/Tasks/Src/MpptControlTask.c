#include "common.h"
#include "BPS_Tasks.h"
#include "overrides.h"
#include "charge.h"
#include "TPEE_Utils.h"

#define MPPT_CAN_DELAY_MS 10u

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
        disableAllMPPTs(MPPT_CAN_DELAY_MS);
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
            disableAllMPPTs(MPPT_CAN_DELAY_MS);
            return;
        }
    }

    // Below the taper point there is no overcharge risk -- pull full power and arm the taper to
    // start from the top next time it is needed.
    if (max_cell_mV < MPPT_TAPER_START_MV)
    {
        s_vlimit_mV = MPPT_MAX_BOOST_MV;
        s_taper_since = 0;
        enableAllMPPTs(MPPT_CAN_DELAY_MS);
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
                                  MPPT_CAN_DELAY_MS);
    enableAllMPPTsConstantVoltage(MPPT_CAN_DELAY_MS);

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
            disableAllMPPTs(MPPT_CAN_DELAY_MS);
        }
    }
    else
    {
        s_taper_since = 0; // current rose again or cell dropped out of band -> restart the dwell timer
    }
}
#endif

static void mppt_drive(void)
{
#if BPS_CMD_CONFIG_ADV_MPPT_CONTROL
    if (overrides_adv_mppt_enabled())
    {
        mppt_drive_advanced();
        return;
    }
#endif
    if (charge_is_enabled())
    {
        enableAllMPPTs(MPPT_CAN_DELAY_MS);
    }
    else
    {
        disableAllMPPTs(MPPT_CAN_DELAY_MS);
    }
}

void Task_Mppt_Control(void *pvParameters)
{
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MPPT_CONTROL_TASK_DELAY_MS));

        mppt_drive();
    }
}
