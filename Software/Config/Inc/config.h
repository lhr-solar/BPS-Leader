/** config.h
 * Config file to hold any aliases/constants referenced by multiple files
 * Not specific to a single board/unit
 */

#pragma once

// Drive-override feature config + relaxed "drive profile" setpoints live here.
#include "drive_profile_config.h"

//--------------------------------------------------------------------------------
// Battery Pack layout
#ifndef NUM_BATTERY_MODULES
#define NUM_BATTERY_MODULES             32      // Number of battery modules
#endif

#ifndef NUM_TEMPERATURE_SENSORS
#define NUM_TEMPERATURE_SENSORS         NUM_BATTERY_MODULES      // Number of temperature sensors
#endif

#ifndef NUM_VOLTAGE_SENSORS
#define NUM_VOLTAGE_SENSORS             NUM_BATTERY_MODULES      // Number of voltage sensors
#endif

#define MODULES_PER_SEGMENT (NUM_BATTERY_MODULES / NUM_VOLTTEMP_BOARDS)

#define NUM_VOLTTEMP_BOARDS              8       // Number of volt temp boards

// volttemp segment voltage 
#define CELL_OVERVOLTAGE_THRESHOLD_MV 4200 // 4.2 V
#define CELL_UNDERVOLTAGE_THRESHOLD_MV 2600 // 2.6 V

// precharge macros
#define PACK_OVERVOLTAGE_THRESHOLD_MV (CELL_OVERVOLTAGE_THRESHOLD_MV*NUM_VOLTAGE_SENSORS) // 134.4 V
#define PACK_UNDERVOLTAGE_THRESHOLD_MV (CELL_UNDERVOLTAGE_THRESHOLD_MV*NUM_VOLTAGE_SENSORS) //  83.2 V

// battery segment temp voltage
#define OVERTEMP_THRESHOLD_CHARGING_MC 55000 // 55 C
#define OVERTEMP_THRESHOLD_DISCHARGING_MC 70000 // 70 C

// Charging Thresholds
#define CELL_CHARGING_VOLTAGE_THRESHOLD_MV 4150   // 4.15 V
#define CELL_CHARGING_TEMP_THRESHOLD_MC 50000 // 50 C

// current threshold to determine if battery is charging (negative number is charging, positive is discharging)
#define CHARGING_THRESHOLD_MA (-50) // -50 mA

// Charge-enable anti-oscillation. Charge disables the instant a cell reaches the charge voltage/temp
// limit, but RE-enabling requires the max cell to first fall a hysteresis band BELOW the limit, so a
// cell sitting at the limit can't flip charge on/off ("charge complete -> resume" / "cooled ->
// resume"). The minimum-disable dwell is a belt-and-suspenders so charge enable can't rapidly flip
// even if the hysteresis band is crossed quickly.
#define CHARGE_REENABLE_VOLTAGE_HYSTERESIS_MV 100   // re-enable 0.1 V below the charge-voltage cutoff
#define CHARGE_REENABLE_TEMP_HYSTERESIS_MC    2000  // re-enable 2 C below the charge-temp cutoff
#define MIN_CHARGE_DISABLE_TIME_MS            5000  // min time charge stays disabled before re-enable. ponytail: calibrate


// How many bad voltage reads are tolerable when switching states and closing contactors 
#define PRECHARGE_UNDERVOLTAGE_DEBOUNCE_LIMIT 2

// Time allowed for a contactor to physically reach a commanded state (and its auxiliary sense to
// settle) before the contactor monitor faults on the command/sense mismatch. Used as the per-
// contactor sense-verify timer period. A welded/stuck contactor (e.g. a shaded array masking a
// welded array contactor with zero current) is caught here regardless of current.
//
// Budget from the contactor datasheet (operate 30ms / release 10ms max @ 23C):
//   ~2x derating for coil-voltage/temperature/aging (~60ms) + aux-contact lag & bounce (~10ms) +
//   RTOS jitter (1ms tick; timer daemon runs below the monitor tasks) (~10ms) ~= 80ms worst-case
//   healthy settle. 150ms gives ~1.9x margin so a slow-but-healthy contactor never false-faults,
//   while still flagging a stuck contactor within the 200ms monitor cadence / 2000ms IWDG.
// NOTE: applies to ALL four contactors; if the HV contactors are a slower part than the array
// contactor this datasheet describes, split into a per-contactor value.
#define CONTACTOR_CHECK_DELAY_MS 150

// How many consecutive bad reads on a single module before a fault is latched (filters single
// abnormal readings from real faults). Separate voltage/temperature thresholds. Stored per-module
// in a uint8_t histogram in the monitor tasks, so these must stay < 255.
#define VOLT_CONSECUTIVE_FAULT_THRESHOLD 5
#define TEMP_CONSECUTIVE_FAULT_THRESHOLD 5

// Debounce counter behaviour on a GOOD read (see debounce_good_read() in common.h):
//   CLEAR        - reset the counter to 0 on any good read (a sensor oscillating across the
//                  threshold is reset every good read and can never reach the fault threshold).
//   LEAKY_BUCKET - decrement the counter by 1 (saturating at 0) so a sensor that is bad more
//                  often than good still accumulates to the fault threshold. Closes the
//                  "oscillating sensor bypasses the fault" escape path.
// ponytail: decrement is by 1; a perfectly 50/50 oscillation still nets ~0. Bias the
// increment if a tighter guarantee is ever needed.
#define DEBOUNCE_MODE_CLEAR        0
#define DEBOUNCE_MODE_LEAKY_BUCKET 1
#define VOLT_TEMP_DEBOUNCE_MODE    DEBOUNCE_MODE_LEAKY_BUCKET

// Car-CAN telemetry forwarding mode for the VT aggregate arrays. The Leader samples/debounces at
// the fast monitor rate but only forwards to the shared car bus once per VOLT/TEMP_CAN_FORWARD_PERIOD_MS.
//   SNAPSHOT - forward the latest sample at each forward tick (freshest, simplest, zero state)
//   AVERAGE  - forward the block-mean of the samples taken since the last forward tick (smoother
//              telemetry; reuses the samples we already take for debounce). Fault/age/idx stay latest.
// Telemetry-only: the Leader's own fault logic always uses every raw 100ms sample regardless.
#define VT_FORWARD_SNAPSHOT 0
#define VT_FORWARD_AVERAGE  1
#define VT_CAN_FORWARD_MODE VT_FORWARD_AVERAGE

// RTOS hardware watchdog (IWDG): 1 = enable (production), 0 = disable (debug only).
// When enabled, Task_PetWatchdog starts the MCU IWDG and only refreshes it once every
// monitored task has checked in, so a hung/deadlocked task forces an MCU reset. Disabling
// it removes all missed-deadline/hang recovery -- leave enabled unless actively debugging.
#define RTOS_WATCHDOG_ENABLE 1

// interrupt priorites
#define SHT45_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 4)
#define EMC2305_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define FDCAN_NVIC_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3)
#define ADC_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

// Amperes Overcurrent Setpoints: 70A discharge, 38A charge
#define OVERCURRENT_DISCHARGE_THRESHOLD_mA  (68000)
#define OVERCURRENT_CHARGE_THRESHOLD_mA     (-38000)

//--------------------------------------------------------------------------------
// NOTE: BPS override (CAN 0x67/0x69) config + drive-profile setpoints moved to
// drive_profile_config.h (included above).

#define PRE(s)  "\r    "s"  "   // \r removes the filepath and 'note: '#pragma message:...' parts
#define STR(x)  #x
#define XSTR(x) STR(x)

