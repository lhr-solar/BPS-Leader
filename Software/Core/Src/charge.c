#include "charge.h"
#include "Contactors.h"
#include "TPEE_Utils.h"

// short, non-blocking-ish wait for the boost CAN sends
#define BOOST_CAN_DELAY_MS 5u

// charge_enabled: set true only by the precharge task (array precharged + in range).
static volatile bool g_charge_enabled = false;

// escalation: armed when we disable charge while it was active; if charging current
// persists past CHARGE_CURRENT_DETECTION_DELAY_MS we hard fault.
static volatile bool g_escalation_armed = false;
static volatile TickType_t g_escalation_since = 0;

bool charge_is_enabled(void)
{
    return g_charge_enabled;
}

void charge_set_enabled(bool enabled)
{
    g_charge_enabled = enabled;
}

void charge_force_disable(void)
{
    g_charge_enabled = false;
    disableAllMPPTs(BOOST_CAN_DELAY_MS);
}

void charge_arm_escalation(void)
{
    if (!g_escalation_armed)
    {
        g_escalation_armed = true;
        g_escalation_since = xTaskGetTickCount();
    }
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

    // charging current present (negative current = charging)
    if (pack_current_mA < CHARGING_THRESHOLD_MA)
    {
        // Still charging. Escalate only if the charging current persists CONTINUOUSLY for the
        // full delay (the timer is restarted below whenever current returns to a safe level).
        if (Calculate_TimeDifference(xTaskGetTickCount(), g_escalation_since) >=
            pdMS_TO_TICKS(CHARGE_CURRENT_DETECTION_DELAY_MS))
        {
            // The array failed to stop charging after charge was disabled.
            set_faultBit(PACK_OVERCURRENT_CHARGING_FAULT);
        }
    }
    else
    {
        // Charging current has dropped to a safe level. Disarm only once the underlying cause
        // (cell over-voltage / over-temp) has ALSO cleared -- then the array complied and we have
        // fully recovered. If the cause persists, keep the escalation armed but restart the timer
        // so this safe interval doesn't leave a stale elapsed window that spuriously faults the
        // moment current dips again.
        if (charge_limits_recovered())
        {
            g_escalation_armed = false;
        }
        else
        {
            g_escalation_since = xTaskGetTickCount();
        }
    }
}

void array_shutdown(fault_state_t mode, bool soft)
{
    // contactor_set takes a raw-ms wait (it converts internally)
    TickType_t wait = (mode == EMERGENCY) ? 0 : CALLBACK_BLOCKING_TIME_MS;

    // boost disable first so the MPPTs stop pushing current
    disableAllMPPTs(BOOST_CAN_DELAY_MS);

    if (soft)
    {
        vTaskDelay(pdMS_TO_TICKS(FAULT_SHUTDOWN_MPPT_DELAY_MS));
    }

    contactor_set(ARRAY_CONTACTOR, CONTACTOR_OPEN, wait, mode);

    if (soft)
    {
        vTaskDelay(pdMS_TO_TICKS(FAULT_SHUTDOWN_INTERCONTACTOR_MS));
    }

    contactor_set(ARRAY_PRE_CONTACTOR, CONTACTOR_OPEN, wait, mode);
}
