/** config.h
 * Config file to hold any aliases/constants referenced by multiple files
 * Not specific to a single board/unit
 */

#pragma once

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


// How many bad voltage reads are tolerable when switching states and closing contactors 
#define PRECHARGE_UNDERVOLTAGE_DEBOUNCE_LIMIT 2

// interrupt priorites
#define SHT45_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 4)
#define EMC2305_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define FDCAN_NVIC_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3)
#define ADC_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

// Amperes Overcurrent Setpoints: 70A discharge, 38A charge
#define OVERCURRENT_DISCHARGE_THRESHOLD_mA  (68000)
#define OVERCURRENT_CHARGE_THRESHOLD_mA     (-38000)

//--------------------------------------------------------------------------------
// BPS override (CAN 0x67/0x69) + fault-val (0xF) feature

// What the driver/drive override (0x67 BPS_Drive_Override) does when active (1 = enabled).
// Add future override behaviors here as new macros.
#define DRIVE_OVERRIDE_DISABLE_OVERTEMP     1   // suppress cell overtemp faults pack-wide
#define DRIVE_OVERRIDE_VSAG_COMPENSATION    1   // apply voltage-sag compensation to UV limit

// Voltage sag compensation (only while discharging and drive override active):
//   adj_uv_limit_mV = CELL_UNDERVOLTAGE_THRESHOLD_MV - (I_mA * R_mOhm * FoS%) / 100000
#define ESTIMATED_MODULE_RESISTANCE_MOHM    3  // estimated per-module resistance (milliohms)
#define VSAG_COMPENSATION_FOS_PERCENT       80 // factor of safety (percent)

// On startup, defer overridable module faults (cell over/under-voltage, overtemp) this long
// so a module-override message (0x69) has time to arrive before we latch the fault.
#define STARTUP_FAULT_DELAY_MS              1000

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
#define EMERGENCY_SOFT_SHUTDOWN_MODE       SHUTDOWN_MODE_ALWAYS

// Array shutdown used when CHARGING is disabled (cell over charge-voltage / over-temp).
// Governs ONLY the charge-disable case, not emergency shutdowns.
#define ARRAY_SOFT_SHUTDOWN_MODE           SHUTDOWN_MODE_ALWAYS

// If charging current is still present this long after charge is disabled, hard fault.
#define CHARGE_CURRENT_DETECTION_DELAY_MS  500


#define PRE(s)  "\r    "s"  "   // \r removes the filepath and 'note: '#pragma message:...' parts
#define STR(x)  #x
#define XSTR(x) STR(x)

