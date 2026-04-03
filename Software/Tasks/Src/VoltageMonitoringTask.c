// TODO: Volt temps have multiple faults, this does not discriminate. Add discrimination later.

#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "BPSCAN_can_msgs.h"

#define VOLTAGE_CAN_DELAY_MS 10u

#define TAPS_PER_BOARD ( NUM_TEMPERATURE_SENSORS / NUM_VOLTTEMP_BOARDS )

/* Struct definition: 
typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_VoltTemp_Tap_Fault;
    bool is_working;
} bps_voltage_arr_t;
*/

// get first four bits of volt can message, which is id
static const uint8_t volt_id_mask = 0x1F;

// pass in pointer to raw data, return packed structs
static void volt_can_unpack(uint8_t* raw_volt_can_data, bps_voltage_arr_t* volt_can_data) {

    if (raw_volt_can_data == NULL) {
        volt_can_data->BPS_Voltage_Tap_Fault = 1;
        return;
    }

    volt_can_data->BPS_Tap_idx = raw_volt_can_data[0] & volt_id_mask;
    volt_can_data->BPS_Voltage_Tap_Data = raw_volt_can_data[1];
    volt_can_data->BPS_Voltage_Tap_Data |= raw_volt_can_data[2] << 8;
    volt_can_data->BPS_Voltage_Tap_Fault = raw_volt_can_data[3]; 

    toggleHeartbeat();

    // DEBUG 
    printf("id: %u    Voltage: %u    fault: %u\n\r", 
        volt_can_data->BPS_Tap_idx, volt_can_data->BPS_Voltage_Tap_Data, volt_can_data->BPS_Voltage_Tap_Fault);
    
    
}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array 
static void can_recv_all_taps(uint32_t can_msg_ID, bps_voltage_arr_t volt_can_data[], uint8_t starting_index) {

    // can recieve for all 4 voltage taps for each volttemp board
    for (uint8_t i = 0; i < TAPS_PER_BOARD; i++) {

        uint8_t raw_databuffer[CAN_DLC_BPS_VT0_VOLTAGE_ARR] = { 0 };

        // if can recv fails, set the fault bit of the struct on to indicate that this sensor isnt working
        if (bps_can_recv(can_msg_ID, raw_databuffer, CAN_DLC_BPS_VT0_VOLTAGE_ARR, VOLTAGE_CAN_DELAY_MS) != CAN_OK) {
            set_faultBit(BPS_CAN_ERROR);
            volt_can_data[starting_index + i].BPS_Voltage_Tap_Fault = 1;
        }        
        else {
            volt_can_unpack(raw_databuffer, &volt_can_data[starting_index + i]);
        }
    }
}

// calc is short for calculator btw. Just a little bit of slang.
// ts is in mV 
static uint16_t calc_average_voltage(bps_voltage_arr_t volt_can_data[]) {

    uint32_t sum_voltage = 0;
    uint8_t total_num_working_sensors = 0;

    for (uint8_t i = 0; i < NUM_VOLTAGE_SENSORS; i++) {

        if (volt_can_data[i].BPS_Voltage_Tap_Fault == 0) {
            sum_voltage += volt_can_data[i].BPS_Voltage_Tap_Data;
            total_num_working_sensors++;
        }
    }

    // the return in the fault state doesn't mean anything, just to incate to fault now
    if (total_num_working_sensors == 0) {
        // most likely a software fault
        set_faultBit(BATTERY_OVERVOLTAGE_FAULT);
        return 0xFFFF;
    }
    
    return (uint16_t)(sum_voltage / total_num_working_sensors);
}

void Task_Voltage_Monitor(){

    while(1){
        // Delays
        vTaskDelay(pdMS_TO_TICKS(VOLT_MONITOR_TASK_DELAY_MS));

        // array to hold struct packed can data
        bps_voltage_arr_t volt_can_data[NUM_VOLTAGE_SENSORS] = { 0 };

        // ts in mV
        uint16_t avg_voltage = 0;

        /* gets all tap info, packs it into the struct array, magic numbers are array offsets, 
        since each call gets four tap values, need a difference in starting index of 4 between each call */
        can_recv_all_taps(CAN_ID_BPS_VT0_VOLTAGE_ARR, volt_can_data, 0);
        can_recv_all_taps(CAN_ID_BPS_VT1_VOLTAGE_ARR, volt_can_data, 4);
        can_recv_all_taps(CAN_ID_BPS_VT2_VOLTAGE_ARR, volt_can_data, 8);
        can_recv_all_taps(CAN_ID_BPS_VT3_VOLTAGE_ARR, volt_can_data, 12);
        can_recv_all_taps(CAN_ID_BPS_VT4_VOLTAGE_ARR, volt_can_data, 16);
        can_recv_all_taps(CAN_ID_BPS_VT5_VOLTAGE_ARR, volt_can_data, 20);
        can_recv_all_taps(CAN_ID_BPS_VT6_VOLTAGE_ARR, volt_can_data, 24);
        can_recv_all_taps(CAN_ID_BPS_VT7_VOLTAGE_ARR, volt_can_data, 28);

        avg_voltage = calc_average_voltage(volt_can_data);

        // DEBUG
        printf("avg_voltage: %u.%u\r\n", avg_voltage / 1000, avg_voltage % 1000);


        for (uint8_t i = 0; i < NUM_VOLTAGE_SENSORS; i++) {
            if (volt_can_data[i].BPS_Voltage_Tap_Data > CELL_OVERVOLTAGE_THRESHOLD_MV) {
                set_faultBit(BATTERY_OVERVOLTAGE_FAULT);
            }
            if (volt_can_data[i].BPS_Voltage_Tap_Data < CELL_UNDERVOLTAGE_THRESHOLD_MV) {
                set_faultBit(BATTERY_UNDERVOLTAGE_FAULT);
            }
        }

        // Set event group bit
        // xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
        //                     VOLT_MONITOR_DONE);         /* The bits being set. */
    }
    
}