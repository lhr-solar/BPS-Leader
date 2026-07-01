// TODO: Volttemps have multiple faults, this does not discriminate. Add discrimination later.
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
#define VOLTAGE_CAN_DELAY_MS 10u

// Per-tap RX wait while draining the BPS bus each cycle. Kept small so a missing/late board can't
// stretch the (now faster) monitor period: worst case = NUM_VOLTAGE_SENSORS * this.
#define VOLTAGE_CAN_RECV_TIMEOUT_MS 2u

// Aggregate telemetry to Car CAN is decoupled from the sample rate: we sample/debounce every
// VOLT_MONITOR_TASK_DELAY_MS, but only forward the aggregate array every Nth cycle to keep the
// shared car bus light (300ms / 100ms = every 3rd cycle).
#define VOLT_CAN_FORWARD_PERIOD_MS 300u
#define VOLT_CAN_FORWARD_DECIMATION (VOLT_CAN_FORWARD_PERIOD_MS / VOLT_MONITOR_TASK_DELAY_MS)

// a mask of all 1's to compare against the volt sensor watchdog bitmap to ensure every bit is set (all info received)
#define VOLT_TAPS_ALL_DATA 0xFFFFFFFF

#define VOLT_TAPS_PER_BOARD (NUM_VOLTAGE_SENSORS / NUM_VOLTTEMP_BOARDS)

#define VOLT_WATCHDOG_TIMEOUT_MS 1000

// get first five bits of volt can message, which is id
#define VOLT_ID_MASK 0x1F

// Printf period macros
#define VOLT_LOOP_PRINTF_DELAY_MS 2000
#define VOLT_PRINTF_COUNTER (VOLT_LOOP_PRINTF_DELAY_MS / VOLT_MONITOR_TASK_DELAY_MS)

// array to hold struct packed can data
bps_voltage_aggregate_arr_t volt_can_data[NUM_VOLTAGE_SENSORS] = {0};

// number of consecutive voltage faults before latching module fault (shared voltage/temp counter)
_Static_assert(VOLT_CONSECUTIVE_FAULT_THRESHOLD < 255, "VOLT_CONSECUTIVE_FAULT_THRESHOLD must be less than 255 since the histogram is an array of uint8_t");

// array to store how often a module has consecutively voltage-faulted (over OR under), indexed by module number
uint8_t volt_module_fault_histogram[NUM_VOLTAGE_SENSORS] = {0};

// per-module consecutive count of board-reported BQ/blind-sensor diagnostics (BQ I2C read error,
// tap out-of-bounds). Escalated to BQ_CHIP_FAULT once it reaches the consecutive threshold,
// debounced the same way as the threshold faults so a single transient does not latch.
uint8_t volt_bq_fault_histogram[NUM_VOLTAGE_SENSORS] = {0};

// most-recent min/max single-cell voltage (mV), refreshed once per monitor cycle. Exposed via
// get_max_cell_voltage()/get_min_cell_voltage() so other tasks read one word instead of rescanning
// the shared array (a 32-bit aligned read is atomic on this MCU).
static volatile uint32_t g_max_cell_mV = 0;
static volatile uint32_t g_min_cell_mV = 0;

// pack voltage (sum of all module taps, mV), recomputed once per monitor cycle and published as a
// single 32-bit word. Other tasks (CAN status, fault handler) read this instead of summing the
// shared volt_can_data array mid-update, avoiding torn cross-element reads.
static volatile uint32_t g_pack_voltage_mV = 0;

// bitmap to hold volt sensor watchdog, starts all bits set (good), corresponding bits are cleared if taps don't check in
uint32_t volt_watchdog_bitmap = 0;

uint32_t exposed_volt_watchdog_bitmap = VOLT_TAPS_ALL_DATA;

// watchdog timer
static TimerHandle_t voltage_watchdog_timer;
static StaticTimer_t volt_timer_buffer;

// array that is indexed to get the CAN id for each volltemp
static const uint32_t voltage_can_ids[NUM_VOLTTEMP_BOARDS] = {
    CAN_ID_BPS_VT0_VOLTAGE_ARR,
    CAN_ID_BPS_VT1_VOLTAGE_ARR,
    CAN_ID_BPS_VT2_VOLTAGE_ARR,
    CAN_ID_BPS_VT3_VOLTAGE_ARR,
    CAN_ID_BPS_VT4_VOLTAGE_ARR,
    CAN_ID_BPS_VT5_VOLTAGE_ARR,
    CAN_ID_BPS_VT6_VOLTAGE_ARR,
    CAN_ID_BPS_VT7_VOLTAGE_ARR
};

// pass in pointer to raw data, return packed structs
static uint8_t volt_can_unpack(uint8_t *raw_volt_can_data, bps_voltage_aggregate_arr_t *volt_can_data)
{

    // this function takes payloads from the bps_vt[X]_voltage_arr_t message, and packs it into bps_voltage_aggregate_arr_t

    static TickType_t s_last_rx_times[NUM_VOLTAGE_SENSORS] = {0};

    if (raw_volt_can_data == NULL)
    {
        return 0;
    }

    // byte 0 is the tap ID
    uint8_t tap_index = raw_volt_can_data[0] & VOLT_ID_MASK;

    // tap index is out of bounds
    if (tap_index >= NUM_VOLTAGE_SENSORS)
    {
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
    volt_watchdog_bitmap |= (1U << tap_index);
    portEXIT_CRITICAL();

    return 1;
}

uint32_t get_module_voltage(uint8_t module_num){

    // module number is 0 indexed
    if(module_num >= NUM_BATTERY_MODULES){
        return 69420; // Invalid module number
    }
    return volt_can_data[module_num].BPS_Voltage_Tap_Data;
}

static void volt_can_pack(bps_voltage_aggregate_arr_t volt_can_data, uint8_t *msgArr)
{
    if (msgArr == NULL)
    {
        return;
    }

    // bits [0:4]: Tap index (Byte 0)
    msgArr[0] = (volt_can_data.BPS_Tap_idx & VOLT_ID_MASK);

    // bits [8:23]: Voltage data (Bytes 1-2)
    // Using sizeof(uint16_t) ensures 2 bytes are copied
    memcpy(&msgArr[1], &(volt_can_data.BPS_Voltage_Tap_Data), sizeof(uint16_t));

    // bits [24:31]: Fault data (Byte 3)
    msgArr[3] = volt_can_data.BPS_Voltage_Tap_Fault;

    // bits [32:47]: Tap age (Bytes 4-5)
    // Note: The previous implementation used msgArr[4],
    // but the table confirms bits 32-47 occupy bytes 4 and 5.
    memcpy(&msgArr[4], &(volt_can_data.BPS_Voltage_Tap_Age), sizeof(uint16_t));
}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array
static void can_recv_all_taps(uint32_t can_id_index, bps_voltage_aggregate_arr_t volt_can_data[])
{

    // can recieve for all 4 voltage taps for each volttemp board
    for (uint8_t i = 0; i < VOLT_TAPS_PER_BOARD; i++)
    {

        uint8_t raw_databuffer[CAN_DLC_BPS_VT0_VOLTAGE_ARR] = {0};

        // if can recv fails, set the fault bit of the struct on to indicate that this sensor isnt working
        if (bps_can_recv(voltage_can_ids[can_id_index], raw_databuffer, CAN_DLC_BPS_VT0_VOLTAGE_ARR, VOLTAGE_CAN_RECV_TIMEOUT_MS) == CAN_OK)
        {
            // unpack the BPS voltage message from BPS CAN to the BPS aggregate array message
            volt_can_unpack(raw_databuffer, volt_can_data);
        }
    }
}

// watchdog function that runs when the timer times out
static void vVoltageWatchdogCallback(TimerHandle_t volt_timer)
{
    // Snapshot + reset the shared bitmap inside a short critical section, but run the fault-setting
    // kernel calls (latch_mod_fault / set_faultBit -> event group, semaphore, yield) OUTSIDE it:
    // kernel APIs must not be called with the scheduler/interrupts suspended.
    uint32_t bitmap_snapshot;
    taskENTER_CRITICAL();
    bitmap_snapshot = volt_watchdog_bitmap;
    volt_watchdog_bitmap = 0;
    taskEXIT_CRITICAL();

    // check if every tap has sent voltage information since last timer timeout.
    if (bitmap_snapshot != VOLT_TAPS_ALL_DATA)
    {
        // if one hasn't sent, save bitmap to know which one(s) didn't check in, then set fault bit
        exposed_volt_watchdog_bitmap = bitmap_snapshot;
        latch_mod_fault(get_mod_fault_num(bitmap_snapshot), 0); // Store 0 as the faulted module value since voltage isn't being stored here
        set_faultBit(VOLTTEMP_WATCHDOG_FAULT);
    }
}

uint32_t get_pack_voltage()
{
    // Single-word read of the value published once per monitor cycle (see Task_Voltage_Monitor),
    // not a live sum of the shared array -> no torn cross-element reads.
    return g_pack_voltage_mV;
}

// highest / lowest single-cell voltage in the pack (mV). O(1): returns the value computed once
// per monitor cycle (see the monitor loop), not a fresh array scan.
uint32_t get_max_cell_voltage()
{
    return g_max_cell_mV;
}

uint32_t get_min_cell_voltage()
{
    return g_min_cell_mV;
}

bool get_volt_segment_status(uint8_t segment_num)
{

    // confirm voltage readings are coming in
    if (((exposed_volt_watchdog_bitmap >> (segment_num * MODULES_PER_SEGMENT)) & 0xF) != 0xF)
    {
        return false;
    }

    // make sure no faults
    for (uint8_t tap_num = segment_num * MODULES_PER_SEGMENT; tap_num < (segment_num + 1) * MODULES_PER_SEGMENT; tap_num++)
    {

        if (volt_can_data[tap_num].BPS_Voltage_Tap_Fault != BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OK)
        {
            return false;
        }
    }
    return true;
}

void Task_Voltage_Monitor()
{

    // counter to slow printf messages
    uint32_t volt_printf_debug_counter = 0;

    // decimates Car-CAN aggregate forwarding relative to the (faster) sample/debounce rate
    uint32_t volt_fwd_counter = 0;

    // Monotonic union of every tap that has checked in since boot. Used (instead of the live
    // volt_watchdog_bitmap) for the one-time startup coverage gate so it is immune to the 1000ms
    // watchdog-timer clear -- see where it is OR-ed below.
    uint32_t volt_startup_coverage = 0;

#if (VT_CAN_FORWARD_MODE == VT_FORWARD_AVERAGE)
    // per-tap accumulator + sample count for block-averaging forwarded telemetry over each window
    uint32_t volt_fwd_accum[NUM_VOLTAGE_SENSORS] = {0};
    uint16_t volt_fwd_samples = 0;
#endif

    // Make timer for watchdog
    voltage_watchdog_timer = xTimerCreateStatic(
        "Volt Watchdog",                         /* Name of the timer */
        pdMS_TO_TICKS(VOLT_WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                                  /* auto-reload */
        (void *)0,                               /* Timer ID */
        vVoltageWatchdogCallback,                /* Callback function */
        &volt_timer_buffer                       /* Buffer to hold timer data */
    );

    xTimerStart(voltage_watchdog_timer, 0);

    // Start "OK for charging" asserted so a pack that boots within the charge limits behaves as
    // before (charging allowed below the limit). The re-enable hysteresis above only governs
    // recovery AFTER a charge-limit disable; without this seed a pack booting in the hysteresis
    // band would never enable.
    set_state_bit(VOLT_OK_FOR_CHARGING, STATE_BIT_SET);

    // Seed regen OK too so a pack booting inside the regen hysteresis band isn't stuck NOK.
    set_state_bit(VOLT_OK_FOR_REGEN, STATE_BIT_SET);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        volt_printf_debug_counter++;

        // Delay
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(VOLT_MONITOR_TASK_DELAY_MS));

        // loops through each can ID
        for (uint8_t can_id_index = 0; can_id_index < NUM_VOLTTEMP_BOARDS; can_id_index++)
        {

            // recieve data from all taps from each id
            can_recv_all_taps(can_id_index, volt_can_data);
        }

        // message buffer to hold forward voltage aggregate array can msg
        uint8_t msgBuff[CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR] = {0};

        // flag to determine if voltage is OK (to set state bit)
        bool all_voltage_good = true;

        // min/max single-cell voltage this cycle (bounds checks + the exposed accessors)
        uint32_t max_voltage = 0;
        uint32_t min_voltage = UINT32_MAX;

        // pack-voltage sum published once per cycle (race-free reads), and startup-gate tracking:
        // debounce_clear stays true only if no module is mid-debounce this cycle.
        uint32_t voltage_sum = 0;
        bool debounce_clear = true;

        // Undervoltage limit, optionally lowered by voltage-sag compensation while
        // the drive override is active and discharging (see overrides.c).
        int32_t uv_limit_mV = overrides_adjusted_uv_limit_mV(get_pack_current());

        // Forward the aggregate array to Car CAN only every Nth cycle (telemetry decimation,
        // decoupled from the faster sample/debounce rate so the shared car bus stays light).
        bool forward_now = (volt_fwd_counter == 0);
        volt_fwd_counter = (volt_fwd_counter + 1) % VOLT_CAN_FORWARD_DECIMATION;

#if (VT_CAN_FORWARD_MODE == VT_FORWARD_AVERAGE)
        volt_fwd_samples++;
#endif

        // Loop through every received value
        for (uint8_t i = 0; i < NUM_VOLTAGE_SENSORS; i++)
        {
            // update min/max cell voltage
            if (volt_can_data[i].BPS_Voltage_Tap_Data > max_voltage)
            {
                max_voltage = volt_can_data[i].BPS_Voltage_Tap_Data;
            }
            if (volt_can_data[i].BPS_Voltage_Tap_Data < min_voltage)
            {
                min_voltage = volt_can_data[i].BPS_Voltage_Tap_Data;
            }

            // accumulate the pack-voltage sum published at the end of this cycle
            voltage_sum += volt_can_data[i].BPS_Voltage_Tap_Data;

            // Escalate the board's own BQ/blind-sensor diagnostic. Read it BEFORE the threshold
            // logic below overwrites BPS_Voltage_Tap_Fault: a BQ I2C read error or tap
            // out-of-bounds means this cell reading can't be trusted, so debounce it to BQ_CHIP_FAULT
            // instead of operating blind. (Other board codes here are over/under-voltage, handled below.)
            uint8_t volt_board_fault = volt_can_data[i].BPS_Voltage_Tap_Fault;
            if ((volt_board_fault == BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_BQ_I2C_READ_ERROR) ||
                (volt_board_fault == BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OUT_OF_BOUNDS))
            {
                volt_bq_fault_histogram[volt_can_data[i].BPS_Tap_idx]++;
                if (volt_bq_fault_histogram[volt_can_data[i].BPS_Tap_idx] >= VOLT_CONSECUTIVE_FAULT_THRESHOLD)
                {
                    // A VOLTAGE/ALL module override suppresses this HW fault too (not just the value faults).
                    if (!override_suppress_voltage(volt_can_data[i].BPS_Tap_idx))
                    {
                        all_voltage_good = false;
                        printf("Entering BQ Chip Fault (voltage) for Tap %d: board code %d\r\n", volt_can_data[i].BPS_Tap_idx, volt_board_fault);
                        set_faultBit(BQ_CHIP_FAULT);
                    }
                }
            }
            else
            {
                debounce_good_read(&volt_bq_fault_histogram[volt_can_data[i].BPS_Tap_idx]);
            }

            // if voltage is too high or too low, set relevant fault and set fault bit.
            // The overvoltage ceiling is relaxed while the drive override is active. We only
            // latch once a module has consecutively faulted VOLT_CONSECUTIVE_FAULT_THRESHOLD times
            // (filters single abnormal reads). A matching module override suppresses the fault
            // entirely. Overrides (0x67/0x69) are received during the Task_Init startup window,
            // before this task starts, so they are already in effect on the first check.
            if ((int32_t)volt_can_data[i].BPS_Voltage_Tap_Data > overrides_overvoltage_limit_mV())
            {
                volt_can_data[i].BPS_Voltage_Tap_Fault = BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OVER_VOLTAGE;
                volt_module_fault_histogram[volt_can_data[i].BPS_Tap_idx]++;
                if (volt_module_fault_histogram[volt_can_data[i].BPS_Tap_idx] >= VOLT_CONSECUTIVE_FAULT_THRESHOLD)
                {
                    if (!override_suppress_overvoltage(volt_can_data[i].BPS_Tap_idx))
                    {
                        all_voltage_good = false;
                        printf("Entering Cell Over Voltage Fault for Tap %d: %dmV\r\n", volt_can_data[i].BPS_Tap_idx, volt_can_data[i].BPS_Voltage_Tap_Data);
                        latch_mod_fault(volt_can_data[i].BPS_Tap_idx, volt_can_data[i].BPS_Voltage_Tap_Data); // Store the faulted module value (voltage)
                        set_faultBit(CELL_OVERVOLTAGE_FAULT);
                    }
                }
            }
            else if ((int32_t)volt_can_data[i].BPS_Voltage_Tap_Data < uv_limit_mV)
            {

                volt_can_data[i].BPS_Voltage_Tap_Fault = BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_UNDER_VOLTAGE;
                volt_module_fault_histogram[volt_can_data[i].BPS_Tap_idx]++;
                if (volt_module_fault_histogram[volt_can_data[i].BPS_Tap_idx] >= VOLT_CONSECUTIVE_FAULT_THRESHOLD)
                {
                    if (!override_suppress_undervoltage(volt_can_data[i].BPS_Tap_idx))
                    {
                        all_voltage_good = false;
                        printf("Entering Cell Under Voltage Fault for Tap %d: %dmV\r\n", volt_can_data[i].BPS_Tap_idx, volt_can_data[i].BPS_Voltage_Tap_Data);
                        latch_mod_fault(volt_can_data[i].BPS_Tap_idx, volt_can_data[i].BPS_Voltage_Tap_Data); // Store the faulted module value (voltage)
                        set_faultBit(CELL_UNDERVOLTAGE_FAULT);
                    }
                }
            }
            else
            {
                // voltage in range: relax this module's consecutive-fault counter
                // (leaky-bucket decrement or clear, per VOLT_TEMP_DEBOUNCE_MODE)
                debounce_good_read(&volt_module_fault_histogram[volt_can_data[i].BPS_Tap_idx]);
            }

            // startup gate: this module is only "clear" if neither counter is mid-accumulation
            if ((volt_module_fault_histogram[volt_can_data[i].BPS_Tap_idx] != 0) ||
                (volt_bq_fault_histogram[volt_can_data[i].BPS_Tap_idx] != 0))
            {
                debounce_clear = false;
            }

            // forwarded telemetry value: latest snapshot, or block-average over the window
            bps_voltage_aggregate_arr_t volt_fwd = volt_can_data[i];
#if (VT_CAN_FORWARD_MODE == VT_FORWARD_AVERAGE)
            volt_fwd_accum[i] += volt_can_data[i].BPS_Voltage_Tap_Data;
            if (forward_now)
            {
                volt_fwd.BPS_Voltage_Tap_Data = (uint16_t)(volt_fwd_accum[i] / volt_fwd_samples);
                volt_fwd_accum[i] = 0;
            }
#endif
            volt_can_pack(volt_fwd, msgBuff);
            if (forward_now)
            {
                car_can_send(CAN_ID_BPS_VOLTAGE_AGGREGATE_ARR, msgBuff, CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR, pdMS_TO_TICKS(VOLTAGE_CAN_DELAY_MS));
            }
        }

#if (VT_CAN_FORWARD_MODE == VT_FORWARD_AVERAGE)
        // window complete: reset the block-average sample count for the next forward window
        if (forward_now)
        {
            volt_fwd_samples = 0;
        }
#endif

        // publish this cycle's min/max + pack voltage for cheap, thread-safe single-word reads by other tasks
        g_max_cell_mV = max_voltage;
        g_min_cell_mV = (min_voltage == UINT32_MAX) ? 0 : min_voltage;
        g_pack_voltage_mV = voltage_sum;

        if (volt_printf_debug_counter >= VOLT_PRINTF_COUNTER)
        {
            printf("================================\r\n");
            printf("Voltage values: {");
            for (int j = 0; j < NUM_BATTERY_MODULES; j++)
            {
                printf("%d: %u.%03u V, ", j, volt_can_data[j].BPS_Voltage_Tap_Data / 1000, volt_can_data[j].BPS_Voltage_Tap_Data % 1000);
            }
            printf("}\r\n");
            printf("================================\r\n");


            volt_printf_debug_counter = 0;
        }

        // Charge-enable voltage gate with hysteresis (cutoff relaxed while the drive override is
        // active). Disable the instant any cell reaches the limit; re-enable ("charge complete ->
        // resume") only once the max cell has dropped CHARGE_REENABLE_VOLTAGE_HYSTERESIS_MV below it,
        // so a cell sitting at the limit can't oscillate charge on/off. State holds inside the band.
        int32_t charge_voltage_limit_mV = overrides_charge_limit_voltage_mV();
        if (((int32_t)max_voltage >= charge_voltage_limit_mV) && (get_state_bit(VOLT_OK_FOR_CHARGING) != STATE_BIT_RESET))
        {
            printf("Cell Voltages are NOT ok for charging\r\n");
            set_state_bit(VOLT_OK_FOR_CHARGING, STATE_BIT_RESET);
            charge_force_disable(); // immediate boost off when a cell reaches the charge-voltage limit
        }
        else if (((int32_t)max_voltage < (charge_voltage_limit_mV - CHARGE_REENABLE_VOLTAGE_HYSTERESIS_MV)) && (get_state_bit(VOLT_OK_FOR_CHARGING) != STATE_BIT_SET))
        {
            printf("Cell Voltages are OK for charging\r\n");
            set_state_bit(VOLT_OK_FOR_CHARGING, STATE_BIT_SET);
        }

        // Regen voltage gate with hysteresis (mirror of the charge gate; reported via BPS_Regen_OK,
        // BPS does not actuate regen). Disable at the threshold; re-enable only after the max cell
        // drops REGEN_REENABLE_VOLTAGE_HYSTERESIS_MV below it, so a cell at the limit can't flap regen.
        if (((int32_t)max_voltage >= REGEN_VOLTAGE_THRESHOLD_MV) && (get_state_bit(VOLT_OK_FOR_REGEN) != STATE_BIT_RESET))
        {
            set_state_bit(VOLT_OK_FOR_REGEN, STATE_BIT_RESET);
        }
        else if (((int32_t)max_voltage < (REGEN_VOLTAGE_THRESHOLD_MV - REGEN_REENABLE_VOLTAGE_HYSTERESIS_MV)) && (get_state_bit(VOLT_OK_FOR_REGEN) != STATE_BIT_SET))
        {
            set_state_bit(VOLT_OK_FOR_REGEN, STATE_BIT_SET);
        }

        // Startup contactor-close gate: only mark the monitor "good" once every tap reported this
        // watchdog window (full coverage) AND no module is mid-debounce, so HV can't close on
        // incomplete tap data. This bit is a one-time startup latch (never cleared), so the extra
        // conditions only delay the first close; they don't affect steady-state operation.
        // Accumulate startup coverage monotonically so the one-time VOLTAGE_MONITOR_GOOD latch is
        // immune to the 1000ms watchdog-timer clear of volt_watchdog_bitmap. Reading the live bitmap
        // directly raced that clear: an unlucky alignment left it transiently incomplete, so
        // volt_full_coverage flickered false and stalled the startup contactor-close gate. OR-ing
        // each cycle captures every tap seen since boot regardless of when the watchdog clears.
        volt_startup_coverage |= volt_watchdog_bitmap;
        bool volt_full_coverage = (volt_startup_coverage == VOLT_TAPS_ALL_DATA);
        if (all_voltage_good && volt_full_coverage && debounce_clear && (get_state_bit(VOLTAGE_MONITOR_GOOD) != STATE_BIT_SET))
        {
            if(get_state_bit(VOLT_OK_FOR_CHARGING) == STATE_BIT_SET){
                printf("All module voltages checked and safe\r\n");
            }
            set_state_bit(VOLTAGE_MONITOR_GOOD, STATE_BIT_SET);
        }

        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle, /* The event group being updated. */
                           VOLT_MONITOR_DONE);     /* The bits being set. */
    }
}
