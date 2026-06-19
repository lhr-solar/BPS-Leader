#include "PrechargeTask.h"
#include "FaultHandlerTask.h"
#include "config.h"

// Defined macro is now actually used below
#define PRECHARGE_PRINTF_DEBUG_PERIOD_MS 10000
#define PRECHARGE_PRINTF_DEBUG_COUNTER (PRECHARGE_PRINTF_DEBUG_PERIOD_MS / PRECHARGE_TASK_DELAY_MS)

#define IGNITION_STATUS_TIMEOUT_MS 100

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
    // Use uint64_t to prevent overflow during scaled multiplication of large mV values
    if ((Array_Voltage * VOLTAGE_TOLERANCE_DENOMINATOR) > (Battery_Voltage * VOLTAGE_TOLERANCE_NUMERATOR))
    {
        set_faultBit(ARRAY_GREATER_THAN_BATTERY_FAULT);
    }

    if (Battery_Voltage > PACK_OVERVOLTAGE_THRESHOLD_MV)
    {
        /* BATTERY ABOUT TO GO BOOM */
        set_faultBit(PACK_OVERVOLTAGE_FAULT);
    }

    if ((Battery_Voltage < PACK_UNDERVOLTAGE_THRESHOLD_MV) && (State != PRECHARGE_STATE_IDLE) && (State != PRECHARGE_STATE_FAULT))
    {
        if (State == PRECHARGE_STATE_PRECHARGING) {
            under_voltage_error_count++;
        }
        else {
            /* Battery voltage is too low or battery is disconnected, treat as fault */
            set_faultBit(PACK_UNDERVOLTAGE_FAULT);
        }

        if (under_voltage_error_count > PRECHARGE_UNDERVOLTAGE_DEBOUNCE_LIMIT) {
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
    case PRECHARGE_STATE_FAULT:
        printf("Precharge State: FAULT\r\n");
        break;
    default:
        printf("Precharge State: Unknown\r\n");
        break;
    }
}

can_status_t CarCAN_Recv_Driver_Input(driver_input_status_t *out, TickType_t delay) {
    if (out == NULL) return CAN_EMPTY;

    FDCAN_RxHeaderTypeDef header = {0};
    uint8_t driver_input_rx_data[CAN_DLC_DRIVER_INPUT_STATUS] = {0};

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


void Task_Precharge(void *pvParameters) // Added standard FreeRTOS signature
{
    Init_PrechargeTask();
    printf("Precharge Task Initialized\r\n");

    static Precharge_State_t State = PRECHARGE_STATE_IDLE;   
    uint32_t printDebugCounter = 0;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    // contains the state of the ignition switch
    driver_input_status_t driver_input_status = {0};

    while (1)
    {

        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(PRECHARGE_TASK_DELAY_MS));

        //get the latest driver input status from CAN to check the ignition switch positon
        can_status_t driver_can_status = CarCAN_Recv_Driver_Input(&driver_input_status, pdMS_TO_TICKS(IGNITION_STATUS_TIMEOUT_MS));
       
        // if we don't recieve the driver status in time then turn of all the array contactors and go back to idle
        if (driver_can_status != CAN_OK) {
           State = PRECHARGE_STATE_IDLE;
        }
        // driver status messaged recieve and moved the ignition switch to off
        else if(driver_input_status.Ignition_Array == 0){
            State = PRECHARGE_STATE_IDLE;
        }
        // else the State can remain as is


        // TODO: this sucks, pls fix
        // check this bool from fault handler
        if (system_has_faulted){ 
            State = PRECHARGE_STATE_FAULT;
        }

        ADC_Sense_Result ADC_Result = {0};

        // to let your fault handler safely open contactors.
        if (Read_ADC(ADC_TIMEOUT_MS, &ADC_Result) != ADC_SENSE_OK)
        {
            set_faultBit(ADC_ERROR);
        }

        uint32_t Battery_Voltage = ADC_Result.Battery_Voltage;
        uint32_t Array_Voltage = ADC_Result.Array_Voltage;

        printDebugCounter++;

        switch (State)
        {
            case PRECHARGE_STATE_IDLE:

                // idle runs theoretically with no contactors closed, so cannot check for voltage faults

                // disable both array Contactors
                contactor_set(ARRAY_PRE_CONTACTOR, CONTACTOR_OPEN, CALLBACK_BLOCKING_TIME_MS, NORMAL);
                contactor_set(ARRAY_CONTACTOR, CONTACTOR_OPEN, CALLBACK_BLOCKING_TIME_MS, NORMAL);

                // if the ignition is at array, then we can enable the array contactor in the next iteration
                if (driver_input_status.Ignition_Array) 
                {
                    // TODO: add a check to make sure BPS has been safety checked 
                    State = PRECHARGE_STATE_INITIAL;
                }
                else
                {
                    // if the ignition is not at array, then keep the system in idle
                    State = PRECHARGE_STATE_IDLE;
                }
                break;

            case PRECHARGE_STATE_INITIAL:
                // Double check hardware: Ensure this isn't closing the main positive before precharging!
                if (contactor_set(ARRAY_CONTACTOR, CONTACTOR_CLOSED, CALLBACK_BLOCKING_TIME_MS, NORMAL) != CONTACTOR_OK)
                {
                    set_faultBit(CONTACTOR_ARRAY_FAULT);
                }

                State = PRECHARGE_STATE_PRECHARGING;
                xTimerStart(xPrechargeTimer, 0); // Start the timer
                break;

            case PRECHARGE_STATE_PRECHARGING:
                Fault_Checker(Array_Voltage, Battery_Voltage, State);

                // 1. Evaluate success FIRST
                if ((Array_Voltage * RATIO_SCALE) >= (Battery_Voltage * PRECHARGE_THRESHOLD_90))
                {
                    // stop timer since we did it we precharged
                    xTimerStop(xPrechargeTimer, 0);

                    if (contactor_set(ARRAY_PRE_CONTACTOR, CONTACTOR_CLOSED, CALLBACK_BLOCKING_TIME_MS, NORMAL) != CONTACTOR_OK)
                    {
                        set_faultBit(CONTACTOR_ARRAY_PRE_FAULT);
                    }

                    State = PRECHARGE_STATE_RUN;
                }
                // 2. If not successful yet, check if we ran out of time
                else if (precharge_timeout_expired)
                {
                    set_faultBit(PRECHARGE_TIMEOUT_FAULT);
                    State = PRECHARGE_STATE_FAULT; // Park in idle after faulting
                }
                break;

            case PRECHARGE_STATE_RUN:
                Fault_Checker(Array_Voltage, Battery_Voltage, State);

                // Use 80% threshold for hysteresis
                if ((Array_Voltage * RATIO_SCALE) < (Battery_Voltage * PRECHARGE_THRESHOLD_80))
                {
                    set_faultBit(PRECHARGE_OUT_OF_BOUNDS_FAULT);
                }
                break;

            case PRECHARGE_STATE_FAULT:
                xTimerStop(xPrechargeTimer, 0);
                Fault_Checker(Array_Voltage, Battery_Voltage, State);
                break;

            default:
                break;
        }

        // Use the macro instead of a magic number
        if (printDebugCounter >= PRECHARGE_PRINTF_DEBUG_COUNTER)
        {
            printf("Array: %lu mV | Battery: %lu mV\r\n", Array_Voltage, Battery_Voltage);
            print_Precharge_State(State);
            printDebugCounter = 0;
        }
    }
}