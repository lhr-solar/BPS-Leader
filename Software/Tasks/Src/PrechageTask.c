#include "PrechargeTask.h"
#include "FaultHandlerTask.h"

#define PRECHARGE_PRINTF_DEBUG_PERIOD_MS 10000
#define PRECHARGE_PRINTF_DEBUG_COUNTER (PRECHARGE_PRINTF_DEBUG_PERIOD_MS/PRECHARGE_TASK_DELAY_MS)

/* handle for the Precharge task, defined here */
TaskHandle_t hprecharge_task = NULL;

StaticEventGroup_t xPrechargeEventGroup;
EventGroupHandle_t xPrechargeEventGroup_handle;

StaticTask_t Precharge_Task_Buffer;
StackType_t Precharge_Task_Stack[PRECHARGE_TASK_STACK_SIZE];

void Init_PrechargeTask()
{
    // Event Group init
    xPrechargeEventGroup_handle = xEventGroupCreateStatic(&xPrechargeEventGroup);
    configASSERT(xPrechargeEventGroup_handle); // check if handle is set
    // xEventGroupClearBits(xReadADCEventGroup_handle,    /* The event group being updated. */
    //                      0xFF );                    /* The bits being cleared. */
    debugPrintf_init();
    // Inits ADC & contactors
    ADC_Sense_Init();
    contactor_init();
    LEDs_init();
    
}

void Fault_Checker(uint32_t Array_Voltage, uint32_t Battery_Voltage)
{
    if (Array_Voltage > (Battery_Voltage * VOLTAGE_TOLERANCE_NUMERATOR / VOLTAGE_TOLERANCE_DENOMINATOR))
    {
        // Fault handler
        set_faultBit(ARRAY_GREATER_THAN_BATTERY_FAULT);
    }

    if (Battery_Voltage > OVERVOLTAGE_THRESHOLD_MV)
    {
        /* BATTERY ABOUT TO GO BOOM */
        // Fault handler
        set_faultBit(BATTERY_OVERVOLTAGE_FAULT);
    }

    if (Battery_Voltage < UNDERVOLTAGE_THRESHOLD_MV)
    {
        /* Battery voltage is too low or battery is disconnected, treat as fault */
        // Fault handler
        set_faultBit(BATTERY_UNDERVOLTAGE_FAULT);
    }

    if (contactor_get(ARRAY_CONTACTOR) != contactor_get_command_state(ARRAY_CONTACTOR))
    {
        // Fault handler
        set_faultBit(CONTACTOR_UNEXPECTED_STATE_FAULT);
    }

    if (contactor_get(ARRAY_PRE_CONTACTOR) != contactor_get_command_state(ARRAY_PRE_CONTACTOR))
    {
        // Fault handler
        set_faultBit(CONTACTOR_UNEXPECTED_STATE_FAULT);
    }
}

static void print_Precharge_State(Precharge_State_t State){
    switch (State)
    {
        case PRECHARGE_STATE_INITIAL:
            printf("Precharge State: Initial\r\n");
            break;
        case PRECHARGE_STATE_PRECHARGING:
            printf("Precharge State: Precharging\r\n");
            break;
        case PRECHARGE_STATE_RUN:
            printf("Precharge State: Run\r\n");
            break;
        default:
            printf("Unknown\r\n");
            break;
    }
}

void Task_Precharge()
{
    Init_PrechargeTask();

    printf("all intialized\r\n");

    static Precharge_State_t State = PRECHARGE_STATE_INITIAL;
    static TickType_t Start_Tick = 0;
    static bool toggle = true;

    uint8_t printDebugCounter = 0;

    while (1)
    {

        ADC_Sense_Result ADC_Result = {0};
        if (Read_ADC(ADC_TIMEOUT_MS, &ADC_Result) != ADC_SENSE_OK)
        {
            Error_Handler();
        }

        uint32_t Battery_Voltage = ADC_Result.Battery_Voltage;
        uint32_t Array_Voltage = ADC_Result.Array_Voltage;

        printDebugCounter++;

        switch (State)
        {
            case PRECHARGE_STATE_INITIAL: // Startup state: Closes main contactor and moves to precharging state
                if (contactor_set(ARRAY_CONTACTOR, CLOSED, CALLBACK_BLOCKING_TIME, NORMAL) != SUCCESS)
                {
                    set_faultBit(CONTACTOR_TIMEOUT_FAULT);
                }
                State = PRECHARGE_STATE_PRECHARGING;

                // Start a timer for precharging
                Start_Tick = xTaskGetTickCount();
                break;

            case PRECHARGE_STATE_PRECHARGING: // Precharging state: Waits for battery voltage to reach 90% of array voltage, then closes precharge contactor and moves to run state

                Fault_Checker(Array_Voltage, Battery_Voltage); // Check for faults while precharging, if any fault conditions are met, will call fault handler and not proceed with precharge sequence

                const TickType_t Current_Tick = xTaskGetTickCount(); // Check how long we've been precharging for, fault if not precharged after PRECHARGE_TIMEOUT_MS
                if ((Current_Tick - Start_Tick) > pdMS_TO_TICKS(PRECHARGE_TIMEOUT_MS)) // Faults if precharging takes too long
                {
                    // Check if array voltage is within 90% of battery voltage (precharge complete)
                    if (Array_Voltage * RATIO_SCALE >= Battery_Voltage * PRECHARGE_THRESHOLD_90)
                    {
                        if (contactor_set(ARRAY_PRE_CONTACTOR, CLOSED, CALLBACK_BLOCKING_TIME, false) != SUCCESS)
                        {
                            set_faultBit(CONTACTOR_TIMEOUT_FAULT);
                        }
                        State = PRECHARGE_STATE_RUN;
                    }
                    else
                    {
                        // Precharging took too long
                        set_faultBit(PRECHARGE_TIMEOUT_FAULT);
                    }
                }
                break;
            case PRECHARGE_STATE_RUN: // Run state: Continuously checks that array voltage stays within 80% of battery voltage

                Fault_Checker(Array_Voltage, Battery_Voltage); // Check for faults while precharging, if any fault conditions are met, will call fault handler and not proceed with precharge sequence

                // Use 80% threshold for hysteresis
                if (Array_Voltage * RATIO_SCALE < Battery_Voltage * PRECHARGE_THRESHOLD_80)
                {   
                    
                }
                break;
            default:
                break;
        }

        if(printDebugCounter >= 100){

            // prints battery and array voltage
            printf("Array: %ld mV | Battery: %ld mV\r\n",
               Array_Voltage,
               Battery_Voltage);

            // prints current precharge state
            print_Precharge_State(State);
            printDebugCounter = 0;
        }

        LED_set(CHARGING_LED, State == PRECHARGE_STATE_RUN ? OFF : ON);

        setHeartbeat(toggle);
        toggle = !toggle;
        vTaskDelay(PRECHARGE_TASK_DELAY_MS);
    }
}