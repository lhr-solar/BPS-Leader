#include "charge.h"
#include "Contactors.h"
#include "TPEE_Utils.h"

// short, non-blocking-ish wait for the boost CAN sends
#define BOOST_CAN_DELAY_MS 5u

// charge_enabled: set true only by the precharge task (array precharged + in range).
static volatile bool g_charge_enabled = false;

// Tick of the most recent enabled->disabled transition. Anchors BOTH the minimum charge-disable
// dwell (charge_reenable_allowed, anti-oscillation) and the escalation grace window below.
static volatile TickType_t g_last_disable_tick = 0;

// escalation: armed (by the precharge task) when charge is disabled while it was active. Once the
// grace window from the disable edge elapses, any remaining charging current means the array failed
// to stop -> hard fault.
static volatile bool g_escalation_armed = false;

bool charge_is_enabled(void)
{
    return g_charge_enabled;
}

// Record the disable edge (enabled -> disabled). Both the re-enable dwell and the escalation grace
// window are measured from this instant.
static inline void charge_note_disable_edge(void)
{
    if (g_charge_enabled)
    {
        g_last_disable_tick = xTaskGetTickCount();
    }
}

void charge_set_enabled(bool enabled)
{
    if (!enabled)
    {
        charge_note_disable_edge();
    }
    g_charge_enabled = enabled;
}

void charge_force_disable(void)
{
    charge_note_disable_edge();
    g_charge_enabled = false;
    disableAllMPPTs(BOOST_CAN_DELAY_MS);
}

bool charge_reenable_allowed(void)
{
    return Calculate_TimeDifference(xTaskGetTickCount(), g_last_disable_tick) >=
           pdMS_TO_TICKS(MIN_CHARGE_DISABLE_TIME_MS);
}

void charge_arm_escalation(void)
{
    g_escalation_armed = true;
}

void charge_disarm_escalation(void)
{
    g_escalation_armed = false;
}

// Charge limits have recovered: cell voltage AND temperature are both back within the charge
// window (the same state bits the precharge task uses to decide charging is allowed again).
static inline bool charge_limits_recovered(void)
{
    return (get_state_bit(VOLT_OK_FOR_CHARGING) == STATE_BIT_SET) &&
           (get_state_bit(TEMP_OK_FOR_CHARGING) == STATE_BIT_SET);
}

void charge_check_current(int32_t pack_current_mA)
{
    if (!g_escalation_armed)
    {
        return;
    }

    // Grace window from the disable edge: time for the MPPTs to wind down and the array contactors
    // to open. Once it elapses, ANY charging current means the array failed to disconnect, so we hard
    // fault immediately (no further persistence required). Anchored at the disable edge, so a
    // transient dip in current does not reset it.
    bool grace_over = Calculate_TimeDifference(xTaskGetTickCount(), g_last_disable_tick) >=
                      pdMS_TO_TICKS(CHARGE_CURRENT_DETECTION_DELAY_MS);

    if (pack_current_mA < CHARGING_THRESHOLD_MA)
    {
        // charging current present (negative current = charging)
        if (grace_over)
        {
            // The array failed to stop charging after charge was disabled.
            set_faultBit(PACK_OVERCURRENT_CHARGING_FAULT);
        }
    }
    else if (charge_limits_recovered())
    {
        // No charging current and the cause (cell over-voltage / over-temp) has cleared: the array
        // complied and we have fully recovered, so disarm. A welded array contactor that is masked by
        // a shaded array (zero current) is NOT relied on here -- it is caught independently by the
        // contactor monitor, which faults if a commanded-open contactor is still sensed closed past
        // CONTACTOR_CHECK_DELAY_MS, regardless of array current.
        g_escalation_armed = false;
    }
}

void array_shutdown(fault_state_t mode, bool soft)
{
    // contactor_set takes a raw-ms wait (it converts internally)
    TickType_t wait = (mode == EMERGENCY) ? 0 : CALLBACK_BLOCKING_TIME_MS;

    // boost disable first so the MPPTs stop pushing current
    disableAllMPPTs(BOOST_CAN_DELAY_MS);

    // Only when a soft shutdown is active do we sequence the open: wait FAULT_SHUTDOWN_MPPT_DELAY_MS
    // (100-150ms) for the MPPTs to actually wind down, then open array precharge, wait the
    // inter-contactor gap, then open the main array contactor -- so the contacts never break full
    // solar current. When soft is NOT active, fall through and open both at once (boost disable has
    // already been sent above).
    if (soft)
    {
        vTaskDelay(pdMS_TO_TICKS(FAULT_SHUTDOWN_MPPT_DELAY_MS));
    }

    contactor_set(ARRAY_PRE_CONTACTOR, CONTACTOR_OPEN, wait, mode);

    if (soft)
    {
        vTaskDelay(pdMS_TO_TICKS(FAULT_SHUTDOWN_INTERCONTACTOR_MS));
    }

    contactor_set(ARRAY_CONTACTOR, CONTACTOR_OPEN, wait, mode);
}
