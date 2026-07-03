#include "TPEE_Utils.h"

typedef enum{
    MPPT_A = CAN_ID_MPPT_A_SETMODE,
    MPPT_B = CAN_ID_MPPT_B_SETMODE,
    MPPT_C = CAN_ID_MPPT_C_SETMODE,
} mppt_set_mode_can_ids_t;

// assumes that all the set mode DLC's are the same length for all MPPTs
#define MPPT_SETMODE_DLC CAN_DLC_MPPT_A_SETMODE

#define MPPT_SETMODE_ENABLED 1
#define MPPT_SETMODE_DISABLED 0
#define MPPT_SETMODE_ENABLED_CONSTANT_VOLTAGE 2

// Per-MPPT SetOutputVoltageLimit CAN IDs (mirrors the set-mode id enum above)
typedef enum {
    MPPT_A_VLIMIT = CAN_ID_MPPT_A_SETOUTPUTVOLTAGELIMIT,
    MPPT_B_VLIMIT = CAN_ID_MPPT_B_SETOUTPUTVOLTAGELIMIT,
    MPPT_C_VLIMIT = CAN_ID_MPPT_C_SETOUTPUTVOLTAGELIMIT,
} mppt_set_vlimit_can_ids_t;

// assumes all MPPTs share the same SetOutputVoltageLimit DLC
#define MPPT_SETVLIMIT_DLC CAN_DLC_MPPT_A_SETOUTPUTVOLTAGELIMIT

static can_status_t sendMPPTSetModeCommand(mppt_set_mode_can_ids_t mppt_id, uint8_t mode, TickType_t delay_ms);

static can_status_t sendMPPTSetModeCommand(mppt_set_mode_can_ids_t mppt_id, uint8_t mode, TickType_t delay_ms){
    uint8_t msg[MPPT_SETMODE_DLC] = {0}; 
    msg[0] = mode; // set mode to the desired value

    if(car_can_send(mppt_id, msg, MPPT_SETMODE_DLC, delay_ms) != CAN_OK){
        return CAN_ERR;
    }
    return CAN_OK;
}

can_status_t disableAllMPPTs(TickType_t delay_ms){

    can_status_t status = CAN_OK;
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_A_SETMODE, MPPT_SETMODE_DISABLED, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_B_SETMODE, MPPT_SETMODE_DISABLED, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_C_SETMODE, MPPT_SETMODE_DISABLED, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    return status;
}

can_status_t enableAllMPPTs(TickType_t delay_ms){

    can_status_t status = CAN_OK;
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_A_SETMODE, MPPT_SETMODE_ENABLED, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_B_SETMODE, MPPT_SETMODE_ENABLED, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_C_SETMODE, MPPT_SETMODE_ENABLED, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    return status;

}

can_status_t enableAllMPPTsConstantVoltage(TickType_t delay_ms){

    can_status_t status = CAN_OK;
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_A_SETMODE, MPPT_SETMODE_ENABLED_CONSTANT_VOLTAGE, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_B_SETMODE, MPPT_SETMODE_ENABLED_CONSTANT_VOLTAGE, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(sendMPPTSetModeCommand(CAN_ID_MPPT_C_SETMODE, MPPT_SETMODE_ENABLED_CONSTANT_VOLTAGE, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    return status;

}

static can_status_t sendMPPTOutputVoltageLimit(mppt_set_vlimit_can_ids_t mppt_id, int16_t limit_raw, TickType_t delay_ms){
    uint8_t msg[MPPT_SETVLIMIT_DLC] = {0};
    // int16 little-endian payload
    msg[0] = (uint8_t)(limit_raw & 0xFF);
    msg[1] = (uint8_t)(((uint16_t)limit_raw >> 8) & 0xFF);

    if(car_can_send(mppt_id, msg, MPPT_SETVLIMIT_DLC, delay_ms) != CAN_OK){
        return CAN_ERR;
    }
    return CAN_OK;
}

can_status_t setAllMPPTsOutputVoltageLimit(int16_t limit_raw, TickType_t delay_ms){

    can_status_t status = CAN_OK;
    if(sendMPPTOutputVoltageLimit(MPPT_A_VLIMIT, limit_raw, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(sendMPPTOutputVoltageLimit(MPPT_B_VLIMIT, limit_raw, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(sendMPPTOutputVoltageLimit(MPPT_C_VLIMIT, limit_raw, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    return status;

}
