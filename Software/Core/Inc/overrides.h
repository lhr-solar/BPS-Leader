/** overrides.h
 * Central state for the CAN driver/module override feature.
 *
 * - Drive override   (CAN 0x67 BPS_Override):        single global enable bit.
 * - Module overrides (CAN 0x69 BPS_Module_Override): per-module 2-bit code.
 *
 * State defaults to 0 (no override) and only changes when a CAN message is
 * received. A single writer (CAN status task) updates it; monitor tasks read it.
 * All accessors are lightweight and non-blocking (safe from any task).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

// Per-module override codes (matches CarCAN BPS_Module_Override value table)
typedef enum {
    MODULE_OVERRIDE_NORMAL  = 0,
    MODULE_OVERRIDE_VOLTAGE = 1,
    MODULE_OVERRIDE_TEMP    = 2,
    MODULE_OVERRIDE_ALL     = 3,
} module_override_t;

// ---- Drive override (0x67) ----
void overrides_set_drive(uint8_t enabled);
uint8_t overrides_get_drive(void);

// ---- Module overrides (0x69) ----
// data8 is the raw 8-byte payload (32 modules x 2 bits, little-endian).
void overrides_set_module_raw(const uint8_t *data8);
uint8_t overrides_get_module(uint8_t module_num);

// ---- Ack packing (0x667 / 0x669), reflects current state ----
void overrides_pack_drive_ack(uint8_t *data8);
void overrides_pack_module_ack(uint8_t *data8);

// ---- Fault suppression (combines drive + module overrides) ----
bool override_suppress_overvoltage(uint8_t module_num);
bool override_suppress_undervoltage(uint8_t module_num);
bool override_suppress_overtemp(uint8_t module_num);

// ---- Voltage sag compensation ----
// Returns the (possibly lowered) per-cell undervoltage limit in mV for the given
// pack current (mA, positive = discharging). Only compensates while the drive
// override is active and the pack is discharging; otherwise returns the base limit.
int32_t overrides_adjusted_uv_limit_mV(int32_t pack_current_mA);

// ---- Startup fault grace ----
void overrides_arm_startup_grace(void); // call once after CAN is up
bool startup_fault_grace_active(void);  // true while overridable faults should be deferred
