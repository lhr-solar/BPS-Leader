#include "TPEE_Utils.h"

typedef enum{
    MPPT_A = CAN_ID_MPPT_A_SETMODE,
    MPPT_B = CAN_ID_MPPT_B_SETMODE,
    MPPT_C = CAN_ID_MPPT_C_SETMODE,
} mppt_set_mode_can_ids_t;

// assumes that all the set mode DLC's are the same length for all MPPTs
#define MPPT_SETMODE_DLC CAN_DLC_MPPT_A_SETMODE

can_status_t disableAllMPPTs(TickType_t delay_ms){

    uint8_t msg[MPPT_SETMODE_DLC] = {0}; 
    msg[0] = 0; // set mode to disabled

    can_status_t status = CAN_OK;
    if(car_can_send(CAN_ID_MPPT_A_SETMODE, msg, MPPT_SETMODE_DLC, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(car_can_send(CAN_ID_MPPT_B_SETMODE, msg, MPPT_SETMODE_DLC, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(car_can_send(CAN_ID_MPPT_C_SETMODE, msg, MPPT_SETMODE_DLC, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    return status;
}

can_status_t enableAllMPPTs(TickType_t delay_ms){

    uint8_t msg[MPPT_SETMODE_DLC] = {0}; 
    msg[0] = 1; // set mode to enabled

    can_status_t status = CAN_OK;
    if(car_can_send(CAN_ID_MPPT_A_SETMODE, msg, MPPT_SETMODE_DLC, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(car_can_send(CAN_ID_MPPT_B_SETMODE, msg, MPPT_SETMODE_DLC, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    if(car_can_send(CAN_ID_MPPT_C_SETMODE, msg, MPPT_SETMODE_DLC, delay_ms) != CAN_OK){
        status = CAN_ERR;
    }
    return status;

}
