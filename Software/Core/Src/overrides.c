#include "overrides.h"
#include "common.h"

// 32 modules x 2 bits fit in the 8-byte override payloads
#define OVERRIDE_PAYLOAD_BYTES 8

// Single-writer (CAN status task) state; readers are monitor tasks.
static volatile uint8_t g_drive_override = 0;
static volatile uint8_t g_module_override[NUM_BATTERY_MODULES] = {0};

void overrides_set_drive(uint8_t enabled)
{
    g_drive_override = enabled ? 1u : 0u;
}

uint8_t overrides_get_drive(void)
{
    return g_drive_override;
}

void overrides_set_module_raw(const uint8_t *data8)
{
    if (data8 == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    for (uint8_t m = 0; m < NUM_BATTERY_MODULES; m++)
    {
        g_module_override[m] = (data8[m >> 2] >> ((m & 0x3u) * 2u)) & 0x3u;
    }
    taskEXIT_CRITICAL();
}

uint8_t overrides_get_module(uint8_t module_num)
{
    if (module_num >= NUM_BATTERY_MODULES)
    {
        return MODULE_OVERRIDE_NORMAL;
    }
    return g_module_override[module_num];
}

void overrides_pack_drive_ack(uint8_t *data8)
{
    if (data8 == NULL)
    {
        return;
    }

    for (uint8_t i = 0; i < OVERRIDE_PAYLOAD_BYTES; i++)
    {
        data8[i] = 0;
    }
    data8[0] = g_drive_override & 0x1u;
}

void overrides_pack_module_ack(uint8_t *data8)
{
    if (data8 == NULL)
    {
        return;
    }

    for (uint8_t i = 0; i < OVERRIDE_PAYLOAD_BYTES; i++)
    {
        data8[i] = 0;
    }

    taskENTER_CRITICAL();
    for (uint8_t m = 0; m < NUM_BATTERY_MODULES; m++)
    {
        data8[m >> 2] |= (g_module_override[m] & 0x3u) << ((m & 0x3u) * 2u);
    }
    taskEXIT_CRITICAL();
}

// Drive override globally disables overtemp when configured to do so
static inline bool drive_disables_overtemp(void)
{
    return (DRIVE_OVERRIDE_DISABLE_OVERTEMP != 0) && (g_drive_override != 0);
}

bool override_suppress_overvoltage(uint8_t module_num)
{
    uint8_t o = overrides_get_module(module_num);
    return (o == MODULE_OVERRIDE_VOLTAGE) || (o == MODULE_OVERRIDE_ALL);
}

bool override_suppress_undervoltage(uint8_t module_num)
{
    uint8_t o = overrides_get_module(module_num);
    return (o == MODULE_OVERRIDE_VOLTAGE) || (o == MODULE_OVERRIDE_ALL);
}

bool override_suppress_overtemp(uint8_t module_num)
{
    uint8_t o = overrides_get_module(module_num);
    return drive_disables_overtemp() || (o == MODULE_OVERRIDE_TEMP) || (o == MODULE_OVERRIDE_ALL);
}

int32_t overrides_overtemp_limit_mC(bool charging)
{
    if (g_drive_override != 0)
    {
        return charging ? OVERRIDE_OVERTEMP_THRESHOLD_CHARGING_MC
                        : OVERRIDE_OVERTEMP_THRESHOLD_DISCHARGING_MC;
    }
    return charging ? OVERTEMP_THRESHOLD_CHARGING_MC
                    : OVERTEMP_THRESHOLD_DISCHARGING_MC;
}

int32_t overrides_overvoltage_limit_mV(void)
{
    return (g_drive_override != 0) ? OVERRIDE_CELL_OVERVOLTAGE_THRESHOLD_MV
                                   : CELL_OVERVOLTAGE_THRESHOLD_MV;
}

int32_t overrides_charge_limit_voltage_mV(void)
{
    return (g_drive_override != 0) ? OVERRIDE_CELL_CHARGING_VOLTAGE_THRESHOLD_MV
                                   : CELL_CHARGING_VOLTAGE_THRESHOLD_MV;
}

int32_t overrides_adjusted_uv_limit_mV(int32_t pack_current_mA)
{
    // Base UV floor: relaxed override (discharge) setpoint while overriding, else normal.
    int32_t limit = (g_drive_override != 0) ? OVERRIDE_CELL_UNDERVOLTAGE_THRESHOLD_MV
                                            : CELL_UNDERVOLTAGE_THRESHOLD_MV;

    // Only sag-compensate while overriding and discharging (positive current).
    if ((DRIVE_OVERRIDE_VSAG_COMPENSATION != 0) && (g_drive_override != 0) && (pack_current_mA > 0))
    {
        // adj = uv - I(mA) * R(mOhm) * FoS(%) / 100000   (64-bit math avoids overflow)
        int64_t drop = ((int64_t)pack_current_mA * ESTIMATED_MODULE_RESISTANCE_MOHM *
                        VSAG_COMPENSATION_FOS_PERCENT) /
                       100000;
        limit -= (int32_t)drop;
        if (limit < 0)
        {
            limit = 0;
        }
    }

    return limit;
}

bool shutdown_soft_active(uint8_t mode)
{
    switch (mode)
    {
    case SHUTDOWN_MODE_ALWAYS:
        return true;
    case SHUTDOWN_MODE_OVERRIDE:
        return overrides_get_drive() != 0;
    case SHUTDOWN_MODE_NEVER:
    default:
        return false;
    }
}
