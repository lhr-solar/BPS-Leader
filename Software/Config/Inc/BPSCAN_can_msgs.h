#pragma once

#include <stdint.h>

/* ================= CAN ID Macros ================= */

#define CAN_ID_BPS_STATUS 0x1
#define CAN_ID_BPS_VOLTAGE_ARR_0 0x2
#define CAN_ID_BPS_VOLTAGE_ARR_1 0x3
#define CAN_ID_BPS_VOLTAGE_ARR_2 0x4
#define CAN_ID_BPS_VOLTAGE_ARR_3 0x5
#define CAN_ID_BPS_VOLTAGE_ARR_4 0x6
#define CAN_ID_BPS_VOLTAGE_ARR_5 0x7
#define CAN_ID_BPS_VOLTAGE_ARR_6 0x8
#define CAN_ID_BPS_VOLTAGE_ARR_7 0x9
#define CAN_ID_BPS_PACK_CURRENT 0xA
#define CAN_ID_BPS_TEMPERATURE_ARR_0 0x10
#define CAN_ID_BPS_TEMPERATURE_ARR_1 0x11
#define CAN_ID_BPS_TEMPERATURE_ARR_2 0x12
#define CAN_ID_BPS_TEMPERATURE_ARR_3 0x13
#define CAN_ID_BPS_TEMPERATURE_ARR_4 0x14
#define CAN_ID_BPS_TEMPERATURE_ARR_5 0x15
#define CAN_ID_BPS_TEMPERATURE_ARR_6 0x16
#define CAN_ID_BPS_TEMPERATURE_ARR_7 0x17
#define CAN_ID_BPS_PRECHARGE_VOLTAGES 0x20

/* ================= CAN Length Macros ================= */

#define CAN_DLC_BPS_STATUS 7
#define CAN_DLC_BPS_VOLTAGE_ARR_0 4
#define CAN_DLC_BPS_VOLTAGE_ARR_1 4
#define CAN_DLC_BPS_VOLTAGE_ARR_2 4
#define CAN_DLC_BPS_VOLTAGE_ARR_3 4
#define CAN_DLC_BPS_VOLTAGE_ARR_4 4
#define CAN_DLC_BPS_VOLTAGE_ARR_5 4
#define CAN_DLC_BPS_VOLTAGE_ARR_6 4
#define CAN_DLC_BPS_VOLTAGE_ARR_7 4
#define CAN_DLC_BPS_PACK_CURRENT 5
#define CAN_DLC_BPS_TEMPERATURE_ARR_0 7
#define CAN_DLC_BPS_TEMPERATURE_ARR_1 7
#define CAN_DLC_BPS_TEMPERATURE_ARR_2 7
#define CAN_DLC_BPS_TEMPERATURE_ARR_3 7
#define CAN_DLC_BPS_TEMPERATURE_ARR_4 7
#define CAN_DLC_BPS_TEMPERATURE_ARR_5 7
#define CAN_DLC_BPS_TEMPERATURE_ARR_6 7
#define CAN_DLC_BPS_TEMPERATURE_ARR_7 7
#define CAN_DLC_BPS_PRECHARGE_VOLTAGES 6


/* ================= Value Table Enums ================= */

typedef enum {
    BPS_STATUS_BPS_FAULT_DISCHARGING_OVERCURRENT = 16,
    BPS_STATUS_BPS_FAULT_CHARGING_OVERCURRENT = 15,
    BPS_STATUS_BPS_FAULT_ESTOP_3 = 14,
    BPS_STATUS_BPS_FAULT_ESTOP_2 = 13,
    BPS_STATUS_BPS_FAULT_ESTOP_1 = 12,
    BPS_STATUS_BPS_FAULT_ARRAY_PCHG_CONTACTOR_SENSE = 11,
    BPS_STATUS_BPS_FAULT_ARRAY_CONTACTOR_SENSE = 10,
    BPS_STATUS_BPS_FAULT_HV_MINUS_CONTACTOR_SENSE = 9,
    BPS_STATUS_BPS_FAULT_HV_PLUS_CONTACTOR_SENSE = 8,
    BPS_STATUS_BPS_FAULT_WATCHDOG = 7,
    BPS_STATUS_BPS_FAULT_ARRAY_PRECHARGE_TIMEOUT = 6,
    BPS_STATUS_BPS_FAULT_ELCON = 5,
    BPS_STATUS_BPS_FAULT_OVERTEMPERATURE = 4,
    BPS_STATUS_BPS_FAULT_REGEN = 3,
    BPS_STATUS_BPS_FAULT_UNDERVOLTAGE = 2,
    BPS_STATUS_BPS_FAULT_OVERVOLTAGE = 1,
    BPS_STATUS_BPS_FAULT_NO_FAULT = 0,
} bps_status_bps_fault_e;

/* ================= Message Structs ================= */

typedef struct {
    uint8_t BPS_Fault;
    uint8_t BPS_Charge_OK;
    uint8_t BPS_Regen_OK;
    uint8_t HV_Plus_Contactor_State;
    uint8_t HV_Minus_Contactor_State;
    uint8_t Array_Contactor_State;
    uint8_t Array_Precharge_Contactor_State;
    uint32_t Main_Battery_Voltage;
    int16_t Main_Battery_Avg_Temperature;
} bps_status_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_BQ_Fault;
} bps_voltage_arr_0_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_BQ_Fault;
} bps_voltage_arr_1_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_BQ_Fault;
} bps_voltage_arr_2_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_BQ_Fault;
} bps_voltage_arr_3_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_BQ_Fault;
} bps_voltage_arr_4_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_BQ_Fault;
} bps_voltage_arr_5_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_BQ_Fault;
} bps_voltage_arr_6_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_BQ_Fault;
} bps_voltage_arr_7_t;

typedef struct {
    int32_t Main_Battery_Current;
    uint16_t Main_Battery_Current_RawV;
} bps_pack_current_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint8_t BPS_Temperature_Tap_Fault;
    int32_t BPS_Temperature_Tap_Data;
    uint16_t BPS_Temperature_Tap_RawV;
} bps_temperature_arr_0_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint8_t BPS_Temperature_Tap_Fault;
    int32_t BPS_Temperature_Tap_Data;
    uint16_t BPS_Temperature_Tap_RawV;
} bps_temperature_arr_1_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint8_t BPS_Temperature_Tap_Fault;
    int32_t BPS_Temperature_Tap_Data;
    uint16_t BPS_Temperature_Tap_RawV;
} bps_temperature_arr_2_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint8_t BPS_Temperature_Tap_Fault;
    int32_t BPS_Temperature_Tap_Data;
    uint16_t BPS_Temperature_Tap_RawV;
} bps_temperature_arr_3_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint8_t BPS_Temperature_Tap_Fault;
    int32_t BPS_Temperature_Tap_Data;
    uint16_t BPS_Temperature_Tap_RawV;
} bps_temperature_arr_4_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint8_t BPS_Temperature_Tap_Fault;
    int32_t BPS_Temperature_Tap_Data;
    uint16_t BPS_Temperature_Tap_RawV;
} bps_temperature_arr_5_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint8_t BPS_Temperature_Tap_Fault;
    int32_t BPS_Temperature_Tap_Data;
    uint16_t BPS_Temperature_Tap_RawV;
} bps_temperature_arr_6_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint8_t BPS_Temperature_Tap_Fault;
    int32_t BPS_Temperature_Tap_Data;
    uint16_t BPS_Temperature_Tap_RawV;
} bps_temperature_arr_7_t;

typedef struct {
    uint32_t Precharge_Battery_Voltage;
    uint32_t Precharge_Array_Voltage;
} bps_precharge_voltages_t;

