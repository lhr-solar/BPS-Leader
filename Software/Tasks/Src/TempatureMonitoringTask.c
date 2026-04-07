// TODO: Volt temps have multiple faults, this does not discriminate. Add discrimination later.
// TODO: CAN QueueSet solution

#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"


#define TEMPERATURE_CAN_DELAY_MS 10u

#define TAPS_PER_BOARD (NUM_TEMPERATURE_SENSORS / NUM_VOLTTEMP_BOARDS)

#define ALL_DATA 0xFFFFFFFF

#define WATCHDOG_TIMEOUT_MS 1000

// get first four bits of temp can message, which is id
static const uint8_t temp_id_mask = 0x1F;

// get bits 5:7 of temp can message, which is fault code
static const uint8_t temp_fault_mask = 0xE0;

// watchdog bitmap
uint32_t temp_sensor_bitmap;

static TimerHandle_t temperature_watchdog_timer;
static StaticTimer_t temp_timer_buffer;

// pass in pointer to raw data, packs struct into arr. Returns the tap id.
static void temp_can_unpack(uint8_t *raw_temp_can_data, bps_temperature_aggregate_arr_t *temp_can_data)
{

    // if sensor not sending, skip. Watchdog will err eventually if it doesn't receive more data
    if (raw_temp_can_data == NULL) return;

    uint8_t tap_index = raw_temp_can_data[0] & temp_id_mask;

    temp_can_data[tap_index].BPS_Tap_idx = tap_index;
    temp_can_data[tap_index].BPS_Temperature_Tap_Fault = raw_temp_can_data[0] & temp_fault_mask;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data = raw_temp_can_data[1] << 24;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[2] << 16;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[3] << 8;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[4] << 0;
    // last two bytes are debug stuff I do not care about

    // if no fault, set watchdog
    if (temp_can_data[tap_index].BPS_Temperature_Tap_Fault == 0) {
        portENTER_CRITICAL();
        // set corresponding bit in recv bitmap
        temp_sensor_bitmap |= (1U << tap_index);
        portEXIT_CRITICAL();
    }
}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array
static void can_recv_all_taps(uint32_t can_msg_ID, bps_temperature_aggregate_arr_t temp_can_data[])
{

    // can recieve for all 4 temperature taps for each temptemp board
    for (uint8_t i = 0; i < TAPS_PER_BOARD; i++)
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
    if (temp_sensor_bitmap != ALL_DATA)
    {
        set_faultBit(BPS_CAN_ERROR);
    }
    temp_sensor_bitmap = 0;
}

void Task_Temperature_Monitor()
{

    // array to hold struct packed can data
    bps_temperature_aggregate_arr_t temp_can_data[NUM_TEMPERATURE_SENSORS] = {0};

    // Make timer for watchdog
    temperature_watchdog_timer = xTimerCreateStatic(
        "Temp Watchdog",                    /* Name of the timer */
        pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                             /* auto-reload */
        (void*)0,                          /* Timer ID */
        vTemperatureWatchdogCallback,       /* Callback function */
        &temp_timer_buffer                  /* Buffer to hold timer data */
    );

    xTimerStart(temperature_watchdog_timer, 0);

    while (1)
    {

        // Delays
        vTaskDelay(pdMS_TO_TICKS(TEMP_MONITOR_TASK_DELAY_MS));

        // loops through each can ID
        for (uint8_t can_id = CAN_ID_BPS_VT0_TEMPERATURE_ARR; can_id <= CAN_ID_BPS_VT7_TEMPERATURE_ARR; can_id++) {

            // recieve data from all taps from each id
            can_recv_all_taps(can_id, temp_can_data);
        }

        for (uint8_t i = 0; i < NUM_TEMPERATURE_SENSORS; i++)
        {
            if (temp_can_data[i].BPS_Temperature_Tap_Data > OVERTEMP_THRESHOLD_MC)
            {
                set_faultBit(BATTERY_OVERTEMP_FAULT);
            }
        }

        // Set event group bit
        // xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
        //                     TEMP_MONITOR_DONE);         /* The bits being set. */
    }
}