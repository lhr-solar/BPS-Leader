#include "PrechargeTask.h"
#include "FaultHandlerTask.h"
#include "config.h"
#include "charge.h"
#include "overrides.h"

// Charging is allowed when cell voltage and temperature are within the charge limits
// (VOLT/TEMP_OK_FOR_CHARGING state bits) and there is no active fault.
static inline bool charge_conditions_ok(void)
{
    return (get_state_bit(VOLT_OK_FOR_CHARGING) == STATE_BIT_SET) &&
           (get_state_bit(TEMP_OK_FOR_CHARGING) == STATE_BIT_SET) &&
           (is_fault_set(NUM_FAULTS) == false);
}

// Defined macro is now actually used below
#define PRECHARGE_PRINTF_DEBUG_PERIOD_MS 1000
#define PRECHARGE_PRINTF_DEBUG_COUNTER (PRECHARGE_PRINTF_DEBUG_PERIOD_MS / PRECHARGE_TASK_DELAY_MS)

// Dash sends this message every 100 mS, so wait for double that
#define IGNITION_STATUS_TIMEOUT_MS 200

TaskHandle_t hprecharge_task = NULL;

StaticEventGroup_t xPrechargeEventGroup;
EventGroupHandle_t xPrechargeEventGroup_handle;

// The handle for our software timer
static TimerHandle_t xPrechargeTimer = NULL;
static StaticTimer_t xPrechargeTimer_buffer;

static uint8_t under_voltage_error_count = 0;

// Flag to communicate between the Timer Task and Precharge Task
static volatile bool precharge_timeout_expired = false;

// The callback function executed by the FreeRTOS Timer Daemon
void PrechargeTimeoutCallback(TimerHandle_t xTimer)
{
    precharge_timeout_expired = true;
}

void Init_PrechargeTask(void)
{
    xPrechargeEventGroup_handle = xEventGroupCreateStatic(&xPrechargeEventGroup);
    configASSERT(xPrechargeEventGroup_handle);

    if (ADC_Sense_Init() != ADC_SENSE_OK)
    {
        set_faultBit(ADC_ERROR);
    }
    // Create a One-Shot timer (pdFALSE) that does not auto-reload
    xPrechargeTimer = xTimerCreateStatic(
        "PrechargeTmr",                      // Text name
        pdMS_TO_TICKS(PRECHARGE_TIMEOUT_MS), // Timer period
        pdFALSE,                             // pdFALSE = One-Shot, pdTRUE = Auto-Reload
        (void *) 0,                          // Timer ID (not used here)
        PrechargeTimeoutCallback,             // Callback function
        &xPrechargeTimer_buffer              // buffer for precharge timer
    );
}

// could make another eventgroup to distinguish
void Fault_Checker(uint32_t Array_Voltage, uint32_t Battery_Voltage, Precharge_State_t State)
{
    // Scaled-tolerance compare (array may be at most ~10% above battery). Plain uint32 is enough:
    // the largest product here is ~134 V pack in mV * 22 ~= 3.0e6, far below the uint32 max, so the
    // multiply does not overflow (no 64-bit math is used despite the "large mV" values).
    if ((Array_Voltage * VOLTAGE_TOLERANCE_DENOMINATOR) > (Battery_Voltage * VOLTAGE_TOLERANCE_NUMERATOR))
    {
        set_faultBit(ARRAY_GREATER_THAN_BATTERY_FAULT);
    }

    if (Battery_Voltage > PACK_OVERVOLTAGE_THRESHOLD_MV)
    {
        /* BATTERY ABOUT TO GO BOOM */
        set_faultBit(PACK_OVERVOLTAGE_FAULT);
    }

    // Undervoltage is debounced in ALL active states (PRECHARGING and RUN, finding 11): a single
    // noisy ADC sample below the threshold must not immediately latch a full HV shutdown. Only
    // sustained undervoltage past PRECHARGE_UNDERVOLTAGE_DEBOUNCE_LIMIT consecutive checks faults.
    if ((Battery_Voltage < PACK_UNDERVOLTAGE_THRESHOLD_MV) && (State != PRECHARGE_STATE_IDLE) && (State != PRECHARGE_STATE_FAULT))
    {
        under_voltage_error_count++;

        if (under_voltage_error_count > PRECHARGE_UNDERVOLTAGE_DEBOUNCE_LIMIT) {
            /* Battery voltage is sustained too low or battery is disconnected: treat as fault */
            set_faultBit(PACK_UNDERVOLTAGE_FAULT);
        }
    }
    else under_voltage_error_count = 0;
}

static void print_Precharge_State(Precharge_State_t State)
{
    switch (State)
    {
    case PRECHARGE_STATE_IDLE:
        printf("Precharge State: IDLE\r\n");
        break;
    case PRECHARGE_STATE_INITIAL:
        printf("Precharge State: Initial\r\n");
        break;
    case PRECHARGE_STATE_PRECHARGING:
        printf("Precharge State: Precharging\r\n");
        break;
    case PRECHARGE_STATE_RUN:
        printf("Precharge State: Run\r\n");
        break;
    case PRECHARGE_STATE_CHARGE_DISABLED:
        printf("Precharge State: Charge Disabled\r\n");
        break;
    case PRECHARGE_STATE_FAULT:
        printf("Precharge State: FAULT\r\n");
        break;
    default:
        printf("Precharge State: Unknown\r\n");
        break;
    }
}

static can_status_t CarCAN_Recv_Driver_Input(driver_input_status_t *out, TickType_t delay) {
    if (out == NULL) return CAN_EMPTY;

    FDCAN_RxHeaderTypeDef header = {0};
    uint8_t driver_input_rx_data[CAN_DLC_DRIVER_INPUT_STATUS] = {0};

    // A single recv per cycle is correct here (no drain loop needed): CAN_ID_DRIVER_INPUT_STATUS is
    // registered as a DEPTH-1 CIRCULAR queue (see can3_recv_entries.h), so the rx ISR overwrites the
    // one slot with each new frame. The queue therefore holds only the FRESHEST driver input and can
    // never accumulate stale frames or overflow, no matter how fast the dash sends.
    can_status_t result = can_fd_recv(car_can, CAN_ID_DRIVER_INPUT_STATUS, &header, driver_input_rx_data, delay);

    if (result == CAN_OK) {
        out->Ignition_Array   = !!(driver_input_rx_data[0] & (1U << 0));
        out->Ignition_Motor   = !!(driver_input_rx_data[0] & (1U << 1));
        out->Ignition_Off     = !!(driver_input_rx_data[0] & (1U << 2));
        out->Cruise_Enable    = !!(driver_input_rx_data[0] & (1U << 3));
        out->Cruise_Set       = !!(driver_input_rx_data[0] & (1U << 4));
        out->Gear_Forward     = !!(driver_input_rx_data[0] & (1U << 5));
        out->Gear_Neutral     = !!(driver_input_rx_data[0] & (1U << 6));
        out->Gear_Reverse     = !!(driver_input_rx_data[0] & (1U << 7));

        out->Hazard_Pressed      = !!(driver_input_rx_data[1] & (1U << 0));
        out->Horn_Pressed        = !!(driver_input_rx_data[1] & (1U << 1));
        out->Blinker_Left        = !!(driver_input_rx_data[1] & (1U << 2));
        out->Blinker_Right       = !!(driver_input_rx_data[1] & (1U << 3));
        out->PushToTalk_Pressed  = !!(driver_input_rx_data[1] & (1U << 4));
        out->Regen_Activate      = !!(driver_input_rx_data[1] & (1U << 5));
        out->Regen_Enable        = !!(driver_input_rx_data[1] & (1U << 6));
    }

    return result;
}

static can_status_t CarCAN_Send_Precharge_Voltages(uint32_t battery_voltage, uint32_t array_voltage, TickType_t delay_ms){

    uint8_t msgData[CAN_DLC_BPS_PRECHARGE_VOLTAGES] = {0};
    // Pack battery voltage (bits 0-23)
    msgData[0] = battery_voltage & 0xFF; // bits 0-7
    msgData[1] = (battery_voltage >> 8) & 0xFF; // bits 8-15
    msgData[2] = (battery_voltage >> 16) & 0xFF; // bits 16-23  
    // Pack array voltage (bits 24-47)
    msgData[3] = array_voltage & 0xFF; // bits 24-31
    msgData[4] = (array_voltage >> 8) & 0xFF; // bits 32-39
    msgData[5] = (array_voltage >> 16) & 0xFF; // bits 40-47

    return car_can_send(CAN_ID_BPS_PRECHARGE_VOLTAGES, msgData, CAN_DLC_BPS_PRECHARGE_VOLTAGES, delay_ms);
}


void Task_Precharge(void *pvParameters)
{
    Init_PrechargeTask();

    static Precharge_State_t current_precharge_state = PRECHARGE_STATE_IDLE;   
    uint32_t printDebugCounter = 0;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    // contains the state of the ignition switch
    driver_input_status_t driver_input_status = {0};

    /** Precharge sequence
    // TODO: add info about the precharge sequence here
     */

    while (1)
    {

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(PRECHARGE_TASK_DELAY_MS));

        //get the latest driver input status from CAN to check the ignition switch positon
        can_status_t driver_can_status = CarCAN_Recv_Driver_Input(&driver_input_status, pdMS_TO_TICKS(IGNITION_STATUS_TIMEOUT_MS));
       
        // if we don't recieve the driver status in time then turn of all the array contactors and go back to idle
        if (driver_can_status != CAN_OK) {

            // only print it out once to prevent spamming
            if(current_precharge_state != PRECHARGE_STATE_IDLE){
                printf("Precharge Back To Idle due to CAN timeout\r\n");
            }
            current_precharge_state = PRECHARGE_STATE_IDLE;
        }
        // driver status message recieved and ignition switch is off
        else if(driver_input_status.Ignition_Array == 0){

            if(current_precharge_state != PRECHARGE_STATE_IDLE){
                printf("Precharge Back To Idle due to Ignition Switch Off\r\n");
            }
            current_precharge_state = PRECHARGE_STATE_IDLE;
        }
        // else the State can remain as is
        ADC_Sense_Result ADC_Result = {0};

        // to let your fault handler safely open contactors.
        if (Read_ADC(ADC_TIMEOUT_MS, &ADC_Result) != ADC_SENSE_OK)
        {
            set_faultBit(ADC_ERROR);
        }

        uint32_t Battery_Voltage = ADC_Result.Battery_Voltage;
        uint32_t Array_Voltage = ADC_Result.Array_Voltage;

        printDebugCounter++;

        // if any fault is set, or we are already in fault state, stay in fault state until reset
        if (is_fault_set(NUM_FAULTS) || current_precharge_state == PRECHARGE_STATE_FAULT){ 
            current_precharge_state = PRECHARGE_STATE_FAULT;
        }

        // used to store previous precharge state to see if the state has changed and print it out
        Precharge_State_t previousState = current_precharge_state;

        switch (current_precharge_state)
        {
            case PRECHARGE_STATE_IDLE:

                // idle runs theoretically with no contactors closed, so cannot check for voltage faults

                // not charging while idle
                charge_set_enabled(false);
                charge_disarm_escalation();

                // If either array contactor is still closed we may have just dropped here from RUN
                // because ignition/CAN was lost, with the array carrying full solar current. Spin the
                // MPPTs down (boost disable -> wind-down) BEFORE breaking the contacts instead of
                // opening them under load (which arcs and welds the contacts). array_shutdown sequences
                // boost-disable then opens ARRAY then ARRAY_PRE; once both read open the guard skips it
                // so we don't re-disable the MPPTs every idle cycle.
                if ((contactor_get(ARRAY_CONTACTOR) != CONTACTOR_OPEN) ||
                    (contactor_get(ARRAY_PRE_CONTACTOR) != CONTACTOR_OPEN)) {
                    array_shutdown(NORMAL, shutdown_soft_active(ARRAY_SOFT_SHUTDOWN_MODE));
                }

                // Begin precharge only if ignition is at array, the BPS is safety-checked
                // (HV closed), and the pack is within the charge limits.
                if (driver_input_status.Ignition_Array
                    && (contactor_get(HV_PLUS_CONTACTOR) == CONTACTOR_CLOSED)
                    && (contactor_get(HV_MINUS_CONTACTOR) == CONTACTOR_CLOSED)
                    && charge_conditions_ok())
                {
                    current_precharge_state = PRECHARGE_STATE_INITIAL;
                }
                else
                {
                    current_precharge_state = PRECHARGE_STATE_IDLE;
                }
                break;

            case PRECHARGE_STATE_INITIAL:
                if (contactor_set(ARRAY_CONTACTOR, CONTACTOR_CLOSED, CALLBACK_BLOCKING_TIME_MS, NORMAL) != CONTACTOR_OK)
                {
                    set_faultBit(CONTACTOR_ARRAY_FAULT);
                }

                current_precharge_state = PRECHARGE_STATE_PRECHARGING;
                xTimerStart(xPrechargeTimer, 0); // Start the timer
                break;

            case PRECHARGE_STATE_PRECHARGING:
                Fault_Checker(Array_Voltage, Battery_Voltage, current_precharge_state);

                // Check if array votage is close to battery voltage
                if ((Array_Voltage * mV_TO_V_SCALAR) >= (Battery_Voltage * PRECHARGE_THRESHOLD_90))
                {
                    // stop timer since we did it we precharged
                    xTimerStop(xPrechargeTimer, 0);

                    // precharge is complete
                    if (contactor_set(ARRAY_PRE_CONTACTOR, CONTACTOR_CLOSED, CALLBACK_BLOCKING_TIME_MS, NORMAL) != CONTACTOR_OK)
                    {
                        set_faultBit(CONTACTOR_ARRAY_PRE_FAULT);
                    }

                    current_precharge_state = PRECHARGE_STATE_RUN;
                }

                // Precharge is not complete, so we check to see if it's taken too long
                else if (precharge_timeout_expired)
                {
                    set_faultBit(PRECHARGE_TIMEOUT_FAULT);
                    current_precharge_state = PRECHARGE_STATE_FAULT; // Park in idle after faulting
                }
                break;

            case PRECHARGE_STATE_RUN:
                Fault_Checker(Array_Voltage, Battery_Voltage, current_precharge_state);

                // Use 80% threshold for hysteresis
                if ((Array_Voltage * mV_TO_V_SCALAR) < (Battery_Voltage * PRECHARGE_THRESHOLD_80))
                {
                    set_faultBit(PRECHARGE_OUT_OF_BOUNDS_FAULT);
                }

                if (charge_conditions_ok())
                {
                    // array precharged and pack in range -> charging allowed (CAN status task drives boost)
                    charge_set_enabled(true);
                    charge_disarm_escalation();
                }
                else
                {
                    // cell over charge-voltage / over-temp -> disable charge, soft-shut the array,
                    // and arm the "charge should have stopped" current escalation.
                    printf("Charge disabled: soft array shutdown\r\n");
                    charge_set_enabled(false);
                    charge_arm_escalation();
                    array_shutdown(NORMAL, shutdown_soft_active(ARRAY_SOFT_SHUTDOWN_MODE));
                    current_precharge_state = PRECHARGE_STATE_CHARGE_DISABLED;
                }
                break;

            case PRECHARGE_STATE_CHARGE_DISABLED:

                // array is soft-shut; charging stays disabled until the pack returns to range.
                charge_set_enabled(false);

                // The array contactors MUST stay open the whole time charge is disabled. array_shutdown
                // opened them on entry; re-assert here every cycle so a stray closure can't leave the
                // array connected while charging is disabled.
                if (contactor_get(ARRAY_PRE_CONTACTOR) != CONTACTOR_OPEN) {
                    contactor_set(ARRAY_PRE_CONTACTOR, CONTACTOR_OPEN, CALLBACK_BLOCKING_TIME_MS, NORMAL);
                }
                if (contactor_get(ARRAY_CONTACTOR) != CONTACTOR_OPEN) {
                    contactor_set(ARRAY_CONTACTOR, CONTACTOR_OPEN, CALLBACK_BLOCKING_TIME_MS, NORMAL);
                }

                // Recover (re-precharge the array) once voltage and temp are back within the charge
                // limits (with the re-enable hysteresis applied in the monitor tasks), the minimum
                // disable dwell has elapsed (anti-oscillation), and charging is still commanded.
                if (charge_conditions_ok()
                    && charge_reenable_allowed()
                    && driver_input_status.Ignition_Array
                    && (contactor_get(HV_PLUS_CONTACTOR) == CONTACTOR_CLOSED)
                    && (contactor_get(HV_MINUS_CONTACTOR) == CONTACTOR_CLOSED))
                {
                    charge_disarm_escalation();
                    current_precharge_state = PRECHARGE_STATE_INITIAL;
                }
                break;

            case PRECHARGE_STATE_FAULT:

                // Do NOT open contactors here: the fault handler owns the configured
                // emergency shutdown sequence for all hard faults.
                charge_set_enabled(false);
                charge_disarm_escalation();
                xTimerStop(xPrechargeTimer, 0);
                Fault_Checker(Array_Voltage, Battery_Voltage, current_precharge_state);

                break;

            default:
                break;
        }

        if(current_precharge_state != previousState){
            printf("Precharge State Changed to: ");
            print_Precharge_State(current_precharge_state);
        }

        // print voltages and state every PRECHARGE_PRINTF_DEBUG_PERIOD_MS ms for debugging
        if (printDebugCounter >= PRECHARGE_PRINTF_DEBUG_COUNTER)
        {
            printf("Battery: %lu mV | Array: %lu mV\r\n", Battery_Voltage, Array_Voltage);
            print_Precharge_State(current_precharge_state);
            printDebugCounter = 0;
        }

        CarCAN_Send_Precharge_Voltages(Battery_Voltage, Array_Voltage, PRECHARGE_TASK_DELAY_MS / 2);

        // Check in with the RTOS watchdog (one of the ALL_TASKS_DONE bits).
        xEventGroupSetBits(xWDogEventGroup_handle, PRECHARGE_MONITOR_DONE);

        // MPPT boost enable/disable is driven by charge_enabled in the CAN status task.
    }
}