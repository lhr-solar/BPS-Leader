// TODO: Volt temps have multiple faults, this does not discriminate. Add discrimination.
// TODO: CAN QueueSet solution

#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"
#include "overrides.h"
#include "charge.h"
#include "string.h"

// Car-CAN aggregate TX wait (telemetry forward).
#define TEMPERATURE_CAN_DELAY_MS 10u

// Per-tap RX wait while draining the BPS bus each cycle. Kept small so a missing/late board can't
// stretch the (now faster) monitor period: worst case = NUM_TEMPERATURE_SENSORS * this.
#define TEMPERATURE_CAN_RECV_TIMEOUT_MS 2u

// Aggregate + ADC telemetry to Car CAN is decoupled from the sample rate: we sample/debounce every
// TEMP_MONITOR_TASK_DELAY_MS, but only forward every Nth cycle to keep the shared car bus light
// (300ms / 100ms = every 3rd cycle).
#define TEMP_CAN_FORWARD_PERIOD_MS 300u
#define TEMP_CAN_FORWARD_DECIMATION (TEMP_CAN_FORWARD_PERIOD_MS / TEMP_MONITOR_TASK_DELAY_MS)

#define TEMP_TAPS_PER_BOARD (NUM_TEMPERATURE_SENSORS / NUM_VOLTTEMP_BOARDS)

// a mask of all 1's to compare against the temp sensor watchdog bitmap to ensure every bit is set (all info received)
#define TEMP_TAPS_ALL_DATA 0xFFFFFFFF

// timeout for receiving tap information from CAN
#define TEMP_WATCHDOG_TIMEOUT_MS 1000

// picks the charging vs discharging overtemp threshold (charging state bit set in amperes task);
// returns the relaxed override setpoint while the drive override (0x67) is active
#define get_temp_threshold() overrides_overtemp_limit_mC(get_state_bit(CHARGING_BATT_STATE) == STATE_BIT_SET)

// get first five bits of temp can message, which is id
#define TEMP_ID_MASK 0x1F

// get bits 5:7 of temp can message, which is fault code
#define TEMP_FAULT_MASK 0x7

// defines max value a message frame ID can be before reseting to 0
#define FRAME_ID_MAX 255

// Printf period macros
#define TEMP_LOOP_PRINTF_DELAY_MS 2000
#define TEMP_PRINTF_COUNTER (TEMP_LOOP_PRINTF_DELAY_MS / TEMP_MONITOR_TASK_DELAY_MS)

// number of consecutive otemp faults before latching module fault (shared voltage/temp counter)
_Static_assert(TEMP_CONSECUTIVE_FAULT_THRESHOLD < 255, "TEMP_CONSECUTIVE_FAULT_THRESHOLD must be less than 255 since the histogram is an array of uint8_t");

uint32_t exposed_temperature_watchdog_bitmap = TEMP_TAPS_ALL_DATA;

// array to hold struct packed can data
bps_temperature_aggregate_arr_t temp_can_data[NUM_TEMPERATURE_SENSORS] = {0};
bps_temp_adc_aggregate_arr_t temp_can_data2[NUM_TEMPERATURE_SENSORS] = {0};

// watchdog bitmap
uint32_t temp_watchdog_bitmap = 0;

// bitmap to hold temp sensor watchdog, starts all bits set (good), corresponding bits are cleared if taps don't check in
uint32_t exposed_temp_watchdog_bitmap = TEMP_TAPS_ALL_DATA;


// array to store how often a module has consecutively otemp'd, indexed by module number
uint8_t temp_module_fault_histogram[NUM_TEMPERATURE_SENSORS] = {0};

// per-module consecutive count of board-reported blind-sensor diagnostics (thermistor disconnected,
// shorted to GND or VCC). Escalated to BQ_CHIP_FAULT once it reaches the consecutive threshold so a
// blind temperature sensor trips protection instead of the pack operating on unknown cell temps.
uint8_t temp_sensor_fault_histogram[NUM_TEMPERATURE_SENSORS] = {0};

// average pack temperature (mC), recomputed once per cycle and published as a single word so other
// tasks (CAN status) read a consistent value instead of summing the shared array mid-update.
static volatile int32_t g_avg_temp_mC = 0;

// watchdog timer
static TimerHandle_t temperature_watchdog_timer;
static StaticTimer_t temp_timer_buffer;

// array that is indexed to get the CAN id for each volltemp
static const uint32_t temperature_can_ids[NUM_VOLTTEMP_BOARDS] = {
    CAN_ID_BPS_VT0_TEMPERATURE_ARR,
    CAN_ID_BPS_VT1_TEMPERATURE_ARR,
    CAN_ID_BPS_VT2_TEMPERATURE_ARR,
    CAN_ID_BPS_VT3_TEMPERATURE_ARR,
    CAN_ID_BPS_VT4_TEMPERATURE_ARR,
    CAN_ID_BPS_VT5_TEMPERATURE_ARR,
    CAN_ID_BPS_VT6_TEMPERATURE_ARR,
    CAN_ID_BPS_VT7_TEMPERATURE_ARR};

uint32_t get_module_temperature(uint8_t module_num) {

    // invalid module number, return 69420 to indicate error
    if (module_num >= NUM_TEMPERATURE_SENSORS) {
        return 69420;
    }
    return temp_can_data[module_num].BPS_Temperature_Tap_Data;
}  
// pass in pointer to raw data, packs struct into arr
static uint8_t temp_can_unpack(uint8_t *raw_temp_can_data, bps_temperature_aggregate_arr_t *temp_can_data, bps_temp_adc_aggregate_arr_t *temp_can_data2)
{

    static uint8_t frame_id = 0;

    static TickType_t s_last_rx_times[NUM_TEMPERATURE_SENSORS] = {0};

    // if sensor not sending, skip. Watchdog will err eventually if it doesn't receive more data
    if (raw_temp_can_data == NULL)
        return 0;

    // bits [0:4]: Tap index (Byte 0)
    uint8_t tap_index = raw_temp_can_data[0] & TEMP_ID_MASK;

    if (tap_index >= NUM_TEMPERATURE_SENSORS)
        return 0;

    temp_can_data[tap_index].BPS_Tap_idx = tap_index;

    // bits [8:31]: Temp data (24 bits, Bytes 1-3)
    temp_can_data[tap_index].BPS_Temperature_Tap_Data = raw_temp_can_data[1] << 0;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[2] << 8;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[3] << 16;

    // bits [32:39]: Fault data (8 bits, Byte 4)
    temp_can_data[tap_index].BPS_Temperature_Tap_Fault = raw_temp_can_data[4];

    // bits [40:55]: Raw Voltage data (16 bits, Bytes 5-6)
    temp_can_data2[tap_index].BPS_Tap_idx = tap_index;
    temp_can_data2[tap_index].BPS_Temperature_Tap_ADC = raw_temp_can_data[5] << 0;
    temp_can_data2[tap_index].BPS_Temperature_Tap_ADC |= raw_temp_can_data[6] << 8;

    temp_can_data2[tap_index].FrameID_BPS_Temperature = frame_id;
    temp_can_data[tap_index].FrameID_BPS_Temperature = frame_id;

    TickType_t current_time = xTaskGetTickCount();

    // set the time since last receive
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
    temp_watchdog_bitmap |= (1U << tap_index);
    portEXIT_CRITICAL();

    // update frame id to next value, wrap back to 0 if it hits FRAME_ID_MAX + 1
    frame_id = (frame_id + 1) % (FRAME_ID_MAX + 1);

    return 1;
}

// pack temp aggregate array message
static void temp_can_pack(bps_temperature_aggregate_arr_t temp_can_data, uint8_t *msgArr)
{

    if (msgArr == NULL)
    {
        return;
    }

    // bits [0:4]: Tap index (length 5, byte 0)
    msgArr[0] = (temp_can_data.BPS_Tap_idx & TEMP_ID_MASK); // Assuming TEMP_ID_MASK is 0x1F

    // bits [8:31]: Temp data (length 24, bytes 1-3)
    memcpy(&msgArr[1], &(temp_can_data.BPS_Temperature_Tap_Data), 3UL);

    // bits [32:39]: Fault data (length 8, byte 4)
    msgArr[4] = temp_can_data.BPS_Temperature_Tap_Fault;

    // bits [40:55]: Tap age (length 16, bytes 5-6)
    memcpy(&msgArr[5], &(temp_can_data.BPS_Temperature_Tap_Age), sizeof(uint16_t));

    // bits [56:63]: Frame ID (length 8, byte 7)
    msgArr[7] = temp_can_data.FrameID_BPS_Temperature;
}

// Pack rawV debug temp message
static void temp_can_pack2(bps_temp_adc_aggregate_arr_t temp_can_data, uint8_t *msgArr)
{
    if (msgArr == NULL)
    {
        return;
    }

    // bits [0:4]: Tap index (length 5, byte 0)
    // 0x1F is the hex mask for 5 bits (0b00011111)
    msgArr[0] = (temp_can_data.BPS_Tap_idx & 0x1F);

    // bits [8:23]: Raw Voltage (length 16, bytes 1-2)
    memcpy(&msgArr[1], &(temp_can_data.BPS_Temperature_Tap_ADC), sizeof(uint16_t));

    // bits [24:31]: Frame ID (length 8, byte 3)
    msgArr[3] = temp_can_data.FrameID_BPS_Temperature;
}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array
static void can_recv_all_taps(uint32_t can_id_index, bps_temperature_aggregate_arr_t temp_can_data[], bps_temp_adc_aggregate_arr_t temp_can_data2[])
{
    // can recieve for all 4 temperature taps for each temptemp board
    for (uint8_t i = 0; i < TEMP_TAPS_PER_BOARD; i++)
    {
        uint8_t raw_databuffer[CAN_DLC_BPS_VT0_TEMPERATURE_ARR] = {0};
        // if can fails then message will not be unpacked and the watchdog will trip
        if (bps_can_recv(temperature_can_ids[can_id_index], raw_databuffer, CAN_DLC_BPS_VT0_TEMPERATURE_ARR, TEMPERATURE_CAN_RECV_TIMEOUT_MS) == CAN_OK)
        {
            // Unpacking temp CAN messages in aggregate temp array
            temp_can_unpack(raw_databuffer, temp_can_data, temp_can_data2);
        }
    }
}

// watchdog function that runs when the timer times out
static void vTemperatureWatchdogCallback(TimerHandle_t temp_timer)
{
    // Snapshot + reset the shared bitmap in a short critical section, but run the fault-setting
    // kernel calls (latch_mod_fault / set_faultBit) OUTSIDE it -- kernel APIs must not be called
    // with the scheduler/interrupts suspended.
    uint32_t bitmap_snapshot;
    taskENTER_CRITICAL();
    bitmap_snapshot = temp_watchdog_bitmap;
    temp_watchdog_bitmap = 0;
    taskEXIT_CRITICAL();

    // check if every tap has sent temp information since last timer timeout.
    if (bitmap_snapshot != TEMP_TAPS_ALL_DATA)
    {
        // if one hasn't setn, save bitmap to know which one(s) didn't check in, then set fault bit
        exposed_temperature_watchdog_bitmap = bitmap_snapshot;
        latch_mod_fault(get_mod_fault_num(bitmap_snapshot), 0); // Store 0 as the faulted module value since temperature isn't being stored here
        set_faultBit(VOLTTEMP_WATCHDOG_FAULT);
    }
}

// returns the average temperature of all taps (used in can status)
uint32_t get_avg_temp()
{
    // Single-word read of the value published once per monitor cycle (see Task_Temperature_Monitor),
    // not a live sum of the shared array -> no torn cross-element reads.
    return (uint32_t)g_avg_temp_mC;
}

// checks status of a volttem segment
bool get_temp_segment_status(uint8_t segment_num)
{

    // confirm temperature readings are coming in
    if (((exposed_temp_watchdog_bitmap >> (segment_num * MODULES_PER_SEGMENT)) & 0xF) != 0xF)
    {
        return false;
    }

    // make sure no faults
    for (uint8_t tap_num = segment_num * MODULES_PER_SEGMENT; tap_num < (segment_num + 1) * MODULES_PER_SEGMENT; tap_num++)
    {

        if (temp_can_data[tap_num].BPS_Temperature_Tap_Fault != BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OK || temp_can_data[tap_num].BPS_Temperature_Tap_Data > get_temp_threshold())
        {
            return false;
        }
    }
    return true;
}

void Task_Temperature_Monitor()
{

    uint32_t temp_printf_debug_counter = 0;

    // decimates Car-CAN aggregate forwarding relative to the (faster) sample/debounce rate
    uint32_t temp_fwd_counter = 0;

    // Monotonic union of every tap that has checked in since boot. Used (instead of the live
    // temp_watchdog_bitmap) for the one-time startup coverage gate so it is immune to the 1000ms
    // watchdog-timer clear -- see where it is OR-ed below.
    uint32_t temp_startup_coverage = 0;

#if (VT_CAN_FORWARD_MODE == VT_FORWARD_AVERAGE)
    // per-tap accumulators + sample count for block-averaging forwarded telemetry over each window
    int32_t  temp_fwd_accum[NUM_TEMPERATURE_SENSORS]  = {0}; // sum of temp data (mC)
    uint32_t temp_fwd_accum2[NUM_TEMPERATURE_SENSORS] = {0}; // sum of raw ADC counts
    uint16_t temp_fwd_samples = 0;
#endif

    // Make timer for watchdog
    temperature_watchdog_timer = xTimerCreateStatic(
        "Temp Watchdog",                         /* Name of the timer */
        pdMS_TO_TICKS(TEMP_WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                                  /* auto-reload */
        (void *)0,                               /* Timer ID */
        vTemperatureWatchdogCallback,            /* Callback function */
        &temp_timer_buffer                       /* Buffer to hold timer data */
    );

    // start watchdog timer
    xTimerStart(temperature_watchdog_timer, 0);

    // Start "OK for charging" asserted (see the voltage task for rationale): the re-enable
    // hysteresis only governs recovery after a charge-temp disable, so seed the in-range state.
    set_state_bit(TEMP_OK_FOR_CHARGING, STATE_BIT_SET);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        temp_printf_debug_counter++;

        // Delays
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TEMP_MONITOR_TASK_DELAY_MS));

        // loops through each can ID
        for (uint8_t can_id_index = 0; can_id_index < NUM_VOLTTEMP_BOARDS; can_id_index++)
        {

            // recieve data from all taps from each id
            can_recv_all_taps(can_id_index, temp_can_data, temp_can_data2);
        }

        // msg buff is packed with the temp aggregate message, msgBuff 2 is the packed with rawV debug message. Parity between is insured by frame id
        uint8_t msgBuff[CAN_DLC_BPS_TEMPERATURE_AGGREGATE_ARR] = {0};
        uint8_t msgBuff2[CAN_DLC_BPS_TEMP_ADC_AGGREGATE_ARR] = {0};

        // variable used to keep track of maximum temperature for this specific cycle
        uint32_t max_temp = 0;

        // keeps track if all temps pass checks in order to close contactors
        bool all_temp_good = true;

        // avg-temp sum published once per cycle (race-free reads), and startup-gate tracking:
        // debounce_clear stays true only if no module is mid-debounce this cycle.
        int32_t temp_sum = 0;
        bool debounce_clear = true;

        // Forward the aggregate + ADC arrays to Car CAN only every Nth cycle (telemetry decimation,
        // decoupled from the faster sample/debounce rate so the shared car bus stays light).
        bool forward_now = (temp_fwd_counter == 0);
        temp_fwd_counter = (temp_fwd_counter + 1) % TEMP_CAN_FORWARD_DECIMATION;

#if (VT_CAN_FORWARD_MODE == VT_FORWARD_AVERAGE)
        temp_fwd_samples++;
#endif

        // preforms all checks on temperature data, sets relevant flags
        for (uint8_t i = 0; i < NUM_TEMPERATURE_SENSORS; i++)
        {

            // finds the max temperature reading to use for bounds checking
            if (temp_can_data[i].BPS_Temperature_Tap_Data > max_temp)
            {
                max_temp = temp_can_data[i].BPS_Temperature_Tap_Data;
            }

            // accumulate the avg-temp sum published at the end of this cycle
            temp_sum += temp_can_data[i].BPS_Temperature_Tap_Data;

            // Escalate the board's own blind-sensor diagnostic. Read it BEFORE the overtemp logic
            // below overwrites BPS_Temperature_Tap_Fault: a disconnected/shorted thermistor means
            // this cell's temperature is unknown, so debounce it to BQ_CHIP_FAULT instead of
            // operating blind. (Other board codes here are over/under-temp, handled below.)
            uint8_t temp_board_fault = temp_can_data[i].BPS_Temperature_Tap_Fault;
            if ((temp_board_fault == BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_DISCONNECTED) ||
                (temp_board_fault == BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OUT_OF_BOUNDS_SHORT_TO_GND_) ||
                (temp_board_fault == BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OUT_OF_BOUNDS_SHORT_TO_VCC_))
            {
                temp_sensor_fault_histogram[temp_can_data[i].BPS_Tap_idx]++;
                if (temp_sensor_fault_histogram[temp_can_data[i].BPS_Tap_idx] >= TEMP_CONSECUTIVE_FAULT_THRESHOLD)
                {
                    // A TEMP/ALL module override suppresses this HW fault too (not just the value fault).
                    if (!override_suppress_temp(temp_can_data[i].BPS_Tap_idx))
                    {
                        all_temp_good = false;
                        printf("Entering BQ Chip Fault (temp sensor) for Tap %d: board code %d\r\n", temp_can_data[i].BPS_Tap_idx, temp_board_fault);
                        set_faultBit(BQ_CHIP_FAULT);
                    }
                }
            }
            else
            {
                debounce_good_read(&temp_sensor_fault_histogram[temp_can_data[i].BPS_Tap_idx]);
            }

            // check if max temp is greater than threshold (changes depending on if battery is charging or discharging)
            if (temp_can_data[i].BPS_Temperature_Tap_Data > get_temp_threshold())
            {
                if (get_state_bit(CHARGING_BATT_STATE) == STATE_BIT_SET)
                {
                    temp_can_data[i].BPS_Temperature_Tap_Fault = BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_CHARGE_OVER_TEMPERATURE;
                }
                else
                {
                    temp_can_data[i].BPS_Temperature_Tap_Fault = BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OVER_TEMPERATURE;
                }

                // if this module is otemp'd increment that module's fault count
                temp_module_fault_histogram[temp_can_data[i].BPS_Tap_idx]++;


                // only fault the BPS if a singular module has consecutively otemp'd a certain number of times to filter single abnormal readings from actual faults
                if(temp_module_fault_histogram[temp_can_data[i].BPS_Tap_idx] >= TEMP_CONSECUTIVE_FAULT_THRESHOLD)
                {
                    // A matching override (module temp/all override, or drive override disabling
                    // overtemp) suppresses the fault. Overrides (0x67/0x69) are received during the
                    // Task_Init startup window, before this task starts, so they are already in effect.
                    if (!override_suppress_overtemp(temp_can_data[i].BPS_Tap_idx))
                    {
                        all_temp_good = false;
                        printf("Entering Cell Over Temperature Fault for Tap %d: %ldmC\r\n", temp_can_data[i].BPS_Tap_idx, temp_can_data[i].BPS_Temperature_Tap_Data);
                        // latch this module as one who faulted, set fault bit, and set flag indicating we are not good to close contactors
                        latch_mod_fault(temp_can_data[i].BPS_Tap_idx, temp_can_data[i].BPS_Temperature_Tap_Data);
                        set_faultBit(CELL_OVERTEMP_FAULT);
                    }
                }
            }
            else
            {
                // if temp is good, relax this module's consecutive-fault counter
                // (leaky-bucket decrement or clear, per VOLT_TEMP_DEBOUNCE_MODE)
                debounce_good_read(&temp_module_fault_histogram[temp_can_data[i].BPS_Tap_idx]);
            }

            // startup gate: this module is only "clear" if neither counter is mid-accumulation
            if ((temp_module_fault_histogram[temp_can_data[i].BPS_Tap_idx] != 0) ||
                (temp_sensor_fault_histogram[temp_can_data[i].BPS_Tap_idx] != 0))
            {
                debounce_clear = false;
            }

            // forwarded telemetry: latest snapshot, or block-average over the window (data + ADC only)
            bps_temperature_aggregate_arr_t temp_fwd = temp_can_data[i];
            bps_temp_adc_aggregate_arr_t   temp_fwd2 = temp_can_data2[i];
#if (VT_CAN_FORWARD_MODE == VT_FORWARD_AVERAGE)
            temp_fwd_accum[i]  += temp_can_data[i].BPS_Temperature_Tap_Data;
            temp_fwd_accum2[i] += temp_can_data2[i].BPS_Temperature_Tap_ADC;
            if (forward_now)
            {
                temp_fwd.BPS_Temperature_Tap_Data  = (int32_t)(temp_fwd_accum[i] / (int32_t)temp_fwd_samples);
                temp_fwd2.BPS_Temperature_Tap_ADC = (uint16_t)(temp_fwd_accum2[i] / temp_fwd_samples);
                temp_fwd_accum[i]  = 0;
                temp_fwd_accum2[i] = 0;
            }
#endif
            // pack temp aggregate array and rawV/ADC debug message
            temp_can_pack(temp_fwd, msgBuff);
            temp_can_pack2(temp_fwd2, msgBuff2);

            // forward these two messages to carcan (decimated to TEMP_CAN_FORWARD_PERIOD_MS)
            if (forward_now)
            {
                car_can_send(CAN_ID_BPS_TEMPERATURE_AGGREGATE_ARR, msgBuff, CAN_DLC_BPS_TEMPERATURE_AGGREGATE_ARR, pdMS_TO_TICKS(TEMPERATURE_CAN_DELAY_MS));
                car_can_send(CAN_ID_BPS_TEMP_ADC_AGGREGATE_ARR, msgBuff2, CAN_DLC_BPS_TEMP_ADC_AGGREGATE_ARR, pdMS_TO_TICKS(TEMPERATURE_CAN_DELAY_MS));
            }
        }

#if (VT_CAN_FORWARD_MODE == VT_FORWARD_AVERAGE)
        // window complete: reset the block-average sample count for the next forward window
        if (forward_now)
        {
            temp_fwd_samples = 0;
        }
#endif

        // publish this cycle's average temperature for race-free single-word reads by other tasks
        g_avg_temp_mC = temp_sum / (int32_t)NUM_BATTERY_MODULES;

        if (temp_printf_debug_counter >= TEMP_PRINTF_COUNTER)
        {
            printf("================================\r\n");
            printf("Temperature values: {");
            for (int j = 0; j < NUM_BATTERY_MODULES; j++)
            {
                printf("%d: %lu.%03lu C, ", j, temp_can_data[j].BPS_Temperature_Tap_Data / 1000, temp_can_data[j].BPS_Temperature_Tap_Data % 1000);
            }
            printf("}\r\n");
            printf("================================\r\n");

            temp_printf_debug_counter = 0;
        }

        // Charge-enable temperature gate with hysteresis (mirror of the voltage gate): disable on
        // reaching the charge-temp limit; re-enable only after cooling CHARGE_REENABLE_TEMP_HYSTERESIS_MC
        // below it, so a cell hovering at the limit can't oscillate charge on/off. Holds in the band.
        if ((max_temp >= CELL_CHARGING_TEMP_THRESHOLD_MC) && (get_state_bit(TEMP_OK_FOR_CHARGING) != STATE_BIT_RESET))
        {
            set_state_bit(TEMP_OK_FOR_CHARGING, STATE_BIT_RESET);
            charge_force_disable(); // immediate boost off when a cell reaches the charge-temp limit (over temp)
        }
        else if ((max_temp < (CELL_CHARGING_TEMP_THRESHOLD_MC - CHARGE_REENABLE_TEMP_HYSTERESIS_MC)) && (get_state_bit(TEMP_OK_FOR_CHARGING) != STATE_BIT_SET))
        {
            set_state_bit(TEMP_OK_FOR_CHARGING, STATE_BIT_SET);
        }

        // Regen temp gate (reported to the VCU via BPS_Regen_OK; BPS does not actuate regen)
        set_state_bit(TEMP_OK_FOR_REGEN, (max_temp < REGEN_TEMP_THRESHOLD_MC) ? STATE_BIT_SET : STATE_BIT_RESET);

        // Startup contactor-close gate: only mark the monitor "good" once every tap reported this
        // watchdog window (full coverage) AND no module is mid-debounce, so HV can't close on
        // incomplete tap data. One-time startup latch, so these conditions only delay the first close.
        // Accumulate startup coverage monotonically so the one-time TEMPERATURE_MONITOR_GOOD latch is
        // immune to the 1000ms watchdog-timer clear of temp_watchdog_bitmap. Reading the live bitmap
        // directly raced that clear: an unlucky alignment left it transiently incomplete, so
        // temp_full_coverage flickered false and stalled the startup contactor-close gate. OR-ing
        // each cycle captures every tap seen since boot regardless of when the watchdog clears.
        temp_startup_coverage |= temp_watchdog_bitmap;
        bool temp_full_coverage = (temp_startup_coverage == TEMP_TAPS_ALL_DATA);

        // if all temperature values are within range, and the state bit isn't already set, set state bit indicating we're good to close contactors
        if (all_temp_good && temp_full_coverage && debounce_clear && (get_state_bit(TEMPERATURE_MONITOR_GOOD) != STATE_BIT_SET))
        {
            set_state_bit(TEMPERATURE_MONITOR_GOOD, STATE_BIT_SET);
        }

        // Set event group bit for independent watchdog
        xEventGroupSetBits(xWDogEventGroup_handle, /* The event group being updated. */
                           TEMP_MONITOR_DONE);     /* The bits being set. */
    }
}
