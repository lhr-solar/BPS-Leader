/** drive_profile_config.h
 * Config for the BPS "drive override" feature (CAN 0x67/0x69), the relaxed
 * "drive profile" protection setpoints that apply ONLY while the drive override
 * is active, and the (always-on) regen-allowed gating setpoints.
 *
 * Split out of config.h so all override/profile/regen tuning lives in one place.
 * Included by config.h, so every consumer of config.h sees these defines too.
 */

#pragma once

// What the driver/drive override (0x67 BPS_Drive_Override) does when active (1 = enabled).
// Add future override behaviors here as new macros.
#define DRIVE_OVERRIDE_DISABLE_OVERTEMP     0   // 0: relax overtemp to the OVERRIDE_OVERTEMP_* setpoints below instead of blanket-suppressing
#define DRIVE_OVERRIDE_VSAG_COMPENSATION    1   // apply voltage-sag compensation on top of the override UV setpoint

// Voltage sag compensation (only while discharging and drive override active):
//   adj_uv_limit_mV = OVERRIDE_CELL_UNDERVOLTAGE_THRESHOLD_MV - (I_mA * R_mOhm * FoS%) / 100000
#define ESTIMATED_MODULE_RESISTANCE_MOHM    3  // estimated per-module resistance (milliohms)
#define VSAG_COMPENSATION_FOS_PERCENT       80 // factor of safety (percent)

//--------------------------------------------------------------------------------
// Drive profile setpoints: applied ONLY while the drive override (0x67) is active.
// They intentionally relax the normal limits in config.h so the car can keep running
// in an emergency / limp-home.
// ponytail: starting values only — CALIBRATE to the cell datasheet / risk tolerance.

// Cell overtemp limits while override active (normal: 55C charge / 70C discharge)
#define OVERRIDE_OVERTEMP_THRESHOLD_CHARGING_MC      60000   // 60 C
#define OVERRIDE_OVERTEMP_THRESHOLD_DISCHARGING_MC   80000   // 80 C

// Cell voltage limits while override active:
//   discharge -> undervoltage floor   (normal: 2600 mV)
//   charge    -> overvoltage ceiling  (normal: 4200 mV)
#define OVERRIDE_CELL_UNDERVOLTAGE_THRESHOLD_MV      2550    // 2.55 V (discharge floor)
#define OVERRIDE_CELL_OVERVOLTAGE_THRESHOLD_MV       4250    // 4.25 V (charge ceiling)

// Distinct charge cutoff ("OK for charging") voltage while override active. Kept separate
// from the overvoltage fault ceiling above (normal cutoff: 4150 mV).
#define OVERRIDE_CELL_CHARGING_VOLTAGE_THRESHOLD_MV  4200    // 4.2 V

//--------------------------------------------------------------------------------
// Regen-allowed gating. Reported to the VCU via BPS_Regen_OK in the status message:
// regen is advertised as safe only while the drive override (0x67) is active AND the
// pack's max cell voltage AND max cell temperature are below these setpoints. The BPS
// does not actuate regen, it only advertises whether regen is safe; overcurrent still
// faults regardless.
#define REGEN_VOLTAGE_THRESHOLD_MV   4150    // 4.15 V: regen allowed below, disabled at/above
#define REGEN_TEMP_THRESHOLD_MC      55000   // 55 C: regen allowed below

//--------------------------------------------------------------------------------
// Advanced MPPT control (drive-profile only). Default OFF -> the BPS always uses plain boost
// enable/disable. When ADVANCED_MPPT_CONTROL is enabled AND the drive override (0x67) is active,
// the BPS instead drives the MPPTs with a 3-region strategy. With the macro OFF the drive
// override has no effect on MPPT behavior; the feature can only ever engage while overriding.
#ifndef ADVANCED_MPPT_CONTROL
#define ADVANCED_MPPT_CONTROL 1
#endif

// Regions on the pack's MAX cell voltage (mV). Kept in step with the (override) charge cutoffs:
// taper-start = normal charge cutoff, inhibit = override charge cutoff (OVERRIDE_CELL_CHARGING...).
//   max cell <  MPPT_TAPER_START_MV          -> Region A: full MPPT power tracking (mode 1)
//   MPPT_TAPER_START_MV .. MPPT_INHIBIT_MV   -> Region B: constant-voltage taper (mode 2)
//   max cell >= MPPT_INHIBIT_MV              -> Region C: MPPTs off (mode 0)
#define MPPT_TAPER_START_MV   4150
#define MPPT_INHIBIT_MV       4200

// CV-region bus voltage ceiling the MPPTs hold (they taper current internally as the bus reaches
// it). Pack/bus mV; converted to the MPPT's wire units via the scale below.
// ponytail: 32S ~134.4 V full + headroom for noise / IR rise / cell imbalance.
#define MPPT_MAX_BOOST_MV     136000   // 136.0 V (range ~135.5-136.5 V)

// Wire scaling of the SetOutputVoltageLimit int16 field, from the Open-SEC manual (Packet ID 11):
// 0.01 V/LSB => 10 mV per LSB. So MPPT_MAX_BOOST_MV (136000) is sent as 136000/10 = 13600.
// ponytail/VERIFY: confirm 0.01 V/LSB against the current Open-SEC datasheet before relying on the
// CV ceiling. A wrong scale silently mis-sets the constant-voltage limit (only used while
// ADVANCED_MPPT_CONTROL + the drive override are active). NOT verified here -- no datasheet in repo.
#define MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB 10

//--------------------------------------------------------------------------------
// Sequenced "soft" shutdown timing (limits inductive overvoltage / contactor arcing).
// Emergency order: broadcast fault status -> boost disable -> wait MPPT_DELAY (MPPTs wind
// down) -> open array + array precharge -> wait the remainder so HV+ opens ~HV_DELAY after
// the status TX (motor side has zeroed torque & opened its contactors) -> open HV+ then HV-.
#define FAULT_SHUTDOWN_INTERCONTACTOR_MS   50   // "shortly after" gap within each contactor pair
#define FAULT_SHUTDOWN_MPPT_DELAY_MS       150  // wait after boost disable before opening array
#define FAULT_SHUTDOWN_HV_DELAY_MS         500  // delay from fault-status TX to opening HV+

// 3-state shutdown modes (shared by the two configs below).
#define SHUTDOWN_MODE_NEVER                0    // hard: open contactors immediately
#define SHUTDOWN_MODE_ALWAYS               1    // soft: sequenced open
#define SHUTDOWN_MODE_OVERRIDE             2    // soft only while drive override (0x67) active

// Emergency (hard fault) shutdown. In a soft emergency shutdown the array is ALWAYS
// soft-shut as part of the sequence (this does not depend on ARRAY_SOFT_SHUTDOWN_MODE).
#define EMERGENCY_SOFT_SHUTDOWN_MODE       SHUTDOWN_MODE_OVERRIDE

// Array shutdown used when CHARGING is disabled (cell over charge-voltage / over-temp).
// Governs ONLY the charge-disable case, not emergency shutdowns.
#define ARRAY_SOFT_SHUTDOWN_MODE           SHUTDOWN_MODE_OVERRIDE

// If charging current is still present this long after charge is disabled, hard fault.
#define CHARGE_CURRENT_DETECTION_DELAY_MS  500
