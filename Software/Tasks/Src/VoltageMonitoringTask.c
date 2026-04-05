// TODO: Volt temps have multiple faults, this does not discriminate. Add discrimination later.
// TODO: CAN QueueSet solution


#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"

#define VOLTAGE_CAN_DELAY_MS 10u

#define ALL_DATA 0xFFFFFFFF

#define TAPS_PER_BOARD (NUM_VOLTAGE_SENSORS / NUM_VOLTTEMP_BOARDS)

#define WATCHDOG_TIMEOUT_MS 1000

// get first four bits of volt can message, which is id
static const uint8_t volt_id_mask = 0x1F;

// watchdog bitmap
uint32_t volt_sensor_bitmap;

static TimerHandle_t voltage_watchdog_timer;
static StaticTimer_t volt_timer_buffer;

// pass in pointer to raw data, return packed structs
static void volt_can_unpack(uint8_t *raw_volt_can_data, bps_voltage_aggregate_arr_t *volt_can_data)
{

    if (raw_volt_can_data == NULL) return;


    uint8_t tap_index = raw_volt_can_data[0] & volt_id_mask;

    volt_can_data[tap_index].BPS_Tap_idx = tap_index;
    volt_can_data[tap_index].BPS_Voltage_Tap_Data = raw_volt_can_data[1];
    volt_can_data[tap_index].BPS_Voltage_Tap_Data |= raw_volt_can_data[2] << 8;
    volt_can_data[tap_index].BPS_Voltage_Tap_Fault = raw_volt_can_data[3] << 0;

    // if no fault, set watchdog
    if (volt_can_data[tap_index].BPS_Voltage_Tap_Fault == 0)
    {
        portENTER_CRITICAL();
        // set corresponding bit in recv bitmap
        volt_sensor_bitmap |= (1U << tap_index);
        portEXIT_CRITICAL();
    }
}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array
static void can_recv_all_taps(uint32_t can_msg_ID, bps_voltage_aggregate_arr_t volt_can_data[])
{

    // can recieve for all 4 voltage taps for each volttemp board
    for (uint8_t i = 0; i < TAPS_PER_BOARD; i++)
    {

        uint8_t raw_databuffer[CAN_DLC_BPS_VT0_VOLTAGE_ARR] = {0};

        // if can recv fails, set the fault bit of the struct on to indicate that this sensor isnt working
        if (bps_can_recv(can_msg_ID, raw_databuffer, CAN_DLC_BPS_VT0_VOLTAGE_ARR, VOLTAGE_CAN_DELAY_MS) == CAN_OK)
        {
            volt_can_unpack(raw_databuffer, volt_can_data);
        }
    }
}

static void vVoltageWatchdogCallback(TimerHandle_t volt_timer)
{
    if (volt_sensor_bitmap != ALL_DATA)
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
        pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                             /* auto-reload */
        (void *)0,                          /* Timer ID */
        vVoltageWatchdogCallback,           /* Callback function */
        &volt_timer_buffer                  /* Buffer to hold timer data */
    );

    xTimerStart(voltage_watchdog_timer, 0);

    while (1)
    {
        // Delays
        vTaskDelay(pdMS_TO_TICKS(VOLT_MONITOR_TASK_DELAY_MS));

        // loops through each can ID
        for (uint8_t can_id = CAN_ID_BPS_VT0_VOLTAGE_ARR; can_id <= CAN_ID_BPS_VT7_VOLTAGE_ARR; can_id++)
        {

            // recieve data from all taps from each id
            can_recv_all_taps(can_id, volt_can_data);
        }

        for (uint8_t i = 0; i < NUM_VOLTAGE_SENSORS; i++)
        {
            if (volt_can_data[i].BPS_Voltage_Tap_Data > CELL_OVERVOLTAGE_THRESHOLD_MV)
            {
                set_faultBit(BATTERY_OVERVOLTAGE_FAULT);
            }
            if (volt_can_data[i].BPS_Voltage_Tap_Data < CELL_UNDERVOLTAGE_THRESHOLD_MV)
            {
                set_faultBit(BATTERY_UNDERVOLTAGE_FAULT);
            }
        }

        // Set event group bit
        // xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
        //                     VOLT_MONITOR_DONE);         /* The bits being set. */
    }
}