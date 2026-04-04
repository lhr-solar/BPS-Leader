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
#define NUM_TEMPERATURE_SENSORS         32      // Number of temperature sensors
#endif

#ifndef NUM_VOLTAGE_SENSORS
#define NUM_VOLTAGE_SENSORS             32      // Number of voltage sensors
#endif

#define NUM_VOLTTEMP_BOARDS                 8       // Number of volt temp boards

// precharge macros
#define PACK_OVERVOLTAGE_THRESHOLD_MV 140000 // 140 V
#define PACK_UNDERVOLTAGE_THRESHOLD_MV 80000 // 80.0 V

// volttemp segment voltage 
#define CELL_OVERVOLTAGE_THRESHOLD_MV 4200 // 4.2 V
#define CELL_UNDERVOLTAGE_THRESHOLD_MV 2500 // 2.5 V

// battery segment temp voltage
#define OVERTEMP_THRESHOLD_MC 60000 // 60 C

#define IRQ_BASE_PRIO configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY

// interrupt priorites
#define SHT45_IRQ_PRIO (IRQ_BASE_PRIO + 4)
#define EMC2305_IRQ_PRIO (IRQ_BASE_PRIO + 2)
#define FDCAN_NVIC_PRIO (IRQ_BASE_PRIO + 3)
#define ADC_IRQ_PRIO (IRQ_BASE_PRIO + 1)




#define PRE(s)  "\r    "s"  "   // \r removes the filepath and 'note: '#pragma message:...' parts
#define STR(x)  #x
#define XSTR(x) STR(x)

