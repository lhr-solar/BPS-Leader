// TODO: Volt temps have multiple faults, this does not discriminate. Add discrimination later.
// TODO: CAN QueueSet solution


#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"
#include "string.h"

#define VOLTAGE_CAN_DELAY_MS 10u

#define VOLT_TAPS_ALL_DATA 0xFFFFFFFF

#define VOLT_TAPS_PER_BOARD (NUM_VOLTAGE_SENSORS / NUM_VOLTTEMP_BOARDS)

#define VOLT_WATCHDOG_TIMEOUT_MS 1000

// get first four bits of volt can message, which is id
#define VOLT_ID_MASK 0x1F
    
// watchdog bitmap
uint32_t volt_sensor_bitmap;

static TimerHandle_t voltage_watchdog_timer;
static StaticTimer_t volt_timer_buffer;

// pass in pointer to raw data, return packed structs
static uint8_t volt_can_unpack(uint8_t *raw_volt_can_data, bps_voltage_aggregate_arr_t *volt_can_data) {

    // this function takes payloads from the bps_vt[X]_voltage_arr_t message, and packs it into bps_voltage_aggregate_arr_t

    static TickType_t s_last_rx_times[NUM_VOLTAGE_SENSORS] = {0};

    if (raw_volt_can_data == NULL) {
        return 0;
    }

    // byte 0 is the tap ID
    uint8_t tap_index = raw_volt_can_data[0] & VOLT_ID_MASK;

    // tap index is out of bounds
    if(tap_index >= NUM_VOLTAGE_SENSORS){
        return 0;
    }

    volt_can_data[tap_index].BPS_Tap_idx = tap_index;

    // byte 1 - 2 is the actual tap data
    volt_can_data[tap_index].BPS_Voltage_Tap_Data = raw_volt_can_data[1];
    volt_can_data[tap_index].BPS_Voltage_Tap_Data |= ((uint16_t)raw_volt_can_data[2] << 8);

    // byte 3 is the fault
    // we'll preserve whatever fault is set, and then in later logic we'll change it depending on tap voltage
    volt_can_data[tap_index].BPS_Voltage_Tap_Fault = raw_volt_can_data[3];

    TickType_t current_time = xTaskGetTickCount();

    // If this is the very first message received since boot (timestamp is 0)
    if (s_last_rx_times[tap_index] == 0) 
    {
        volt_can_data[tap_index].BPS_Voltage_Tap_Age = 0; 
    } 
    else 
    {
        // Calculate the delta: Current Time - Last Arrival Time
        volt_can_data[tap_index].BPS_Voltage_Tap_Age = Calculate_TimeDifference(current_time, s_last_rx_times[tap_index]);
    }

    s_last_rx_times[tap_index] = current_time;
    
    // set the volt sensor watchdog bitmap
    portENTER_CRITICAL();
    // set corresponding bit in recv bitmap
    volt_sensor_bitmap |= (1U << tap_index);
    portEXIT_CRITICAL();
    volt_can_data[tap_index].BPS_Tap_Msg_WDog = 0;

    return 1;

}

static void volt_can_pack(bps_voltage_aggregate_arr_t volt_can_data, uint8_t *msgArr){
    if(msgArr == NULL){
        return;
    }

    // bits 0-4 are the tap id
    msgArr[0] = (volt_can_data.BPS_Tap_idx & VOLT_ID_MASK);

    // bit 5 is the watchdog
    msgArr[0] |= (volt_can_data.BPS_Tap_Msg_WDog & 0x01) << 5;

    // bytes 1 and 2 are voltage data
    memcpy(&msgArr[1], &(volt_can_data.BPS_Voltage_Tap_Data), sizeof(uint16_t));

    // byte 3 is the fault
    // data stored in struct a 1:1 mapping of what's sent over CAN
    msgArr[3] = volt_can_data.BPS_Voltage_Tap_Fault;

    // bytes 4 and 5 is the time since last recieve
    memcpy(&msgArr[4], &(volt_can_data.BPS_Voltage_Tap_Age), sizeof(uint16_t));

}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array
static void can_recv_all_taps(uint32_t can_msg_ID, bps_voltage_aggregate_arr_t volt_can_data[])
{

    // can recieve for all 4 voltage taps for each volttemp board
    for (uint8_t i = 0; i < VOLT_TAPS_PER_BOARD; i++)
    {

        uint8_t raw_databuffer[CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR] = {0};

        // if can recv fails, set the fault bit of the struct on to indicate that this sensor isnt working
        if (bps_can_recv(can_msg_ID, raw_databuffer, CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR, VOLTAGE_CAN_DELAY_MS) == CAN_OK)
        {
            // unpack the BPS voltage message from BPS CAN to the BPS aggregate array message
            volt_can_unpack(raw_databuffer, volt_can_data);
        }
    }
}


static void vVoltageWatchdogCallback(TimerHandle_t volt_timer)
{   

    if (volt_sensor_bitmap != VOLT_TAPS_ALL_DATA)
    {
        set_faultBit(BPS_CAN_ERROR);
    }
    volt_sensor_bitmap = 0;
}


void Task_Voltage_Monitor()
{

    // array to hold struct packed can data
    bps_voltage_aggregate_arr_t volt_can_data[NUM_VOLTAGE_SENSORS] = {0};

    // Make timer for watchdog
    voltage_watchdog_timer = xTimerCreateStatic(
        "Volt Watchdog",                    /* Name of the timer */
        pdMS_TO_TICKS(VOLT_WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                             /* auto-reload */
        (void *)0,                          /* Timer ID */
        vVoltageWatchdogCallback,           /* Callback function */
        &volt_timer_buffer                  /* Buffer to hold timer data */
    );

    xTimerStart(voltage_watchdog_timer, 0);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        // Delays
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(VOLT_MONITOR_TASK_DELAY_MS));

        // loops through each can ID
        for (uint8_t can_id = CAN_ID_BPS_VT0_VOLTAGE_ARR; can_id <= CAN_ID_BPS_VT7_VOLTAGE_ARR; can_id++)
        {

            // recieve data from all taps from each id
            can_recv_all_taps(can_id, volt_can_data);
        }

        uint8_t msgBuff[CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR] = {0};
        bool good_state = 1;

        for (uint8_t i = 0; i < NUM_VOLTAGE_SENSORS; i++)
        {
            if (volt_can_data[i].BPS_Voltage_Tap_Data > CELL_OVERVOLTAGE_THRESHOLD_MV)
            {
                volt_can_data[i].BPS_Voltage_Tap_Fault = BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OVER_VOLTAGE;
                
                set_faultBit(BATTERY_OVERVOLTAGE_FAULT);
                good_state = 0;
            }
            else if (volt_can_data[i].BPS_Voltage_Tap_Data < CELL_UNDERVOLTAGE_THRESHOLD_MV)
            {
                volt_can_data[i].BPS_Voltage_Tap_Fault = BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_UNDER_VOLTAGE;
                set_faultBit(BATTERY_UNDERVOLTAGE_FAULT);
                good_state = 0;
            }
            
            // pack data for the  msg
            volt_can_pack(volt_can_data[i], msgBuff); 
            car_can_send(CAN_ID_BPS_VOLTAGE_AGGREGATE_ARR, msgBuff, CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR, pdMS_TO_TICKS(VOLTAGE_CAN_DELAY_MS));
        }
        if (good_state == 1) {
            if (get_task_bit(VOLTAGE_MONITOR) == 0) {
                set_task_bit(VOLTAGE_MONITOR);
            }
        }

        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
                            VOLT_MONITOR_DONE);         /* The bits being set. */
    }
}