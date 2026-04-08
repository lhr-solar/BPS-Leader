// TODO: Volt temps have multiple faults, this does not discriminate. Add discrimination.
// TODO: CAN QueueSet solution
// TODO: Implement Fram ID thing Parthiv was talking about

#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"
#include "string.h"

#define TEMPERATURE_CAN_DELAY_MS 10u

#define TEMP_TAPS_PER_BOARD (NUM_TEMPERATURE_SENSORS / NUM_VOLTTEMP_BOARDS)

#define TEMP_TAPS_ALL_DATA 0xFFFFFFFF

#define TEMP_WATCHDOG_TIMEOUT_MS 1000

#define get_temp_threshold() ((get_state_bit(DISCHARGING_BATT_STATE) == 0) ? OVERTEMP_THRESHOLD_CHARGING_MC : OVERTEMP_THRESHOLD_DISCHARGING_MC)

// get first four bits of temp can message, which is id
static const uint8_t temp_id_mask = 0x1F;

// get bits 5:7 of temp can message, which is fault code
static const uint8_t temp_fault_mask = 0x7;

// watchdog bitmap
uint32_t temp_sensor_bitmap;

static TimerHandle_t temperature_watchdog_timer;
static StaticTimer_t temp_timer_buffer;

// pass in pointer to raw data, packs struct into arr. Returns the tap id.
static uint8_t temp_can_unpack(uint8_t *raw_temp_can_data, bps_temperature_aggregate_arr_t *temp_can_data)
{

    static TickType_t s_last_rx_times[NUM_TEMPERATURE_SENSORS] = {0};

    // if sensor not sending, skip. Watchdog will err eventually if it doesn't receive more data
    if (raw_temp_can_data == NULL)
        return 0;

    uint8_t tap_index = raw_temp_can_data[0] & temp_id_mask;

    if (tap_index >= NUM_TEMPERATURE_SENSORS)
        return 0;

    temp_can_data[tap_index].BPS_Tap_idx = tap_index;
    temp_can_data[tap_index].BPS_Temperature_Tap_Fault = (raw_temp_can_data[0] >> 5) & temp_fault_mask;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data = raw_temp_can_data[1] << 0;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[2] << 8;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[3] << 16;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[4] << 24;

    TickType_t current_time = xTaskGetTickCount();

    // set the time since last recieve
    if (s_last_rx_times[tap_index] == 0)
    {
        temp_can_data[tap_index].BPS_Temperature_Tap_Age = 0;
    }
    else
    {
        // Calculate the delta: Current Time - Last Arrival Time
        temp_can_data[tap_index].BPS_Temperature_Tap_Age = Calculate_TimeDifference(current_time, s_last_rx_times[tap_index]);
    }

    s_last_rx_times[tap_index] = current_time;

    portENTER_CRITICAL();
    // set corresponding bit in recv bitmap
    temp_sensor_bitmap |= (1U << tap_index);
    portEXIT_CRITICAL();

    return 1;
}

static void temp_can_pack(bps_temperature_aggregate_arr_t temp_can_data, uint8_t *msgArr)
{
    if (msgArr == NULL)
    {
        return;
    }

    // bits 0-4 are the tap id
    msgArr[0] = (temp_can_data.BPS_Tap_idx & temp_id_mask);

    // bits 5-7 is the fault
    msgArr[0] |= (temp_can_data.BPS_Temperature_Tap_Fault & 0x7) << 5;

    // bytes 1-4 are temp data
    memcpy(&msgArr[1], &(temp_can_data.BPS_Temperature_Tap_Data), sizeof(uint32_t));

    // bytes 5 and 6 is the time since last recieve
    memcpy(&msgArr[5], &(temp_can_data.BPS_Temperature_Tap_Age), sizeof(uint16_t));

    // TODO: Implement Fram ID thing Parthiv was talking about
    msgArr[7] = 0x67;
}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array
static void can_recv_all_taps(uint32_t can_msg_ID, bps_temperature_aggregate_arr_t temp_can_data[])
{

    // can recieve for all 4 temperature taps for each temptemp board
    for (uint8_t i = 0; i < TEMP_TAPS_PER_BOARD; i++)
    {

        uint8_t raw_databuffer[CAN_DLC_BPS_VT0_TEMPERATURE_ARR] = {0};

        // if can fails then message will not be unpacked and the watchdog will trip
        if (bps_can_recv(can_msg_ID, raw_databuffer, CAN_DLC_BPS_VT0_TEMPERATURE_ARR, TEMPERATURE_CAN_DELAY_MS) == CAN_OK)
        {
            temp_can_unpack(raw_databuffer, temp_can_data);
        }
    }
}

static void vTemperatureWatchdogCallback(TimerHandle_t temp_timer)
{
    printf("%lx\r\n", temp_sensor_bitmap);
    if (temp_sensor_bitmap != TEMP_TAPS_ALL_DATA)
    {
        set_faultBit(BPS_CAN_ERROR);
    }
    temp_sensor_bitmap = 0;
}

void Task_Temperature_Monitor()
{

    // array to hold struct packed can data
    static bps_temperature_aggregate_arr_t temp_can_data[NUM_TEMPERATURE_SENSORS] = {0};

    // Make timer for watchdog
    temperature_watchdog_timer = xTimerCreateStatic(
        "Temp Watchdog",                         /* Name of the timer */
        pdMS_TO_TICKS(TEMP_WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                                  /* auto-reload */
        (void *)0,                               /* Timer ID */
        vTemperatureWatchdogCallback,            /* Callback function */
        &temp_timer_buffer                       /* Buffer to hold timer data */
    );

    xTimerStart(temperature_watchdog_timer, 0);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {

        // Delays
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TEMP_MONITOR_TASK_DELAY_MS));

        // loops through each can ID
        for (uint8_t can_id = CAN_ID_BPS_VT0_TEMPERATURE_ARR; can_id <= CAN_ID_BPS_VT7_TEMPERATURE_ARR; can_id++)
        {

            // recieve data from all taps from each id
            can_recv_all_taps(can_id, temp_can_data);
        }

        uint8_t msgBuff[CAN_DLC_BPS_TEMPERATURE_AGGREGATE_ARR] = {0};

        uint32_t temp_threshold = get_temp_threshold();

        bool good_state = 1;

        for (uint8_t i = 0; i < NUM_TEMPERATURE_SENSORS; i++)
        {
            if (temp_can_data[i].BPS_Temperature_Tap_Data > temp_threshold)
            {
                set_faultBit(BATTERY_OVERTEMP_FAULT);
                good_state = 0;
            }

            temp_can_pack(temp_can_data[i], msgBuff);
            car_can_send(CAN_ID_BPS_TEMPERATURE_AGGREGATE_ARR, msgBuff, CAN_DLC_BPS_TEMPERATURE_AGGREGATE_ARR, pdMS_TO_TICKS(TEMPERATURE_CAN_DELAY_MS));
        }
        if (good_state == 1)
        {
            if (get_task_bit(TEMPERATURE_MONITOR) == 0)
            {
                set_task_bit(TEMPERATURE_MONITOR);
            }
        }

        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle, /* The event group being updated. */
                           TEMP_MONITOR_DONE);     /* The bits being set. */
    }
}