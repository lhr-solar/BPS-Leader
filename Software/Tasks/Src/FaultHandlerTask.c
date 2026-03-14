#include "FaultHandlerTask.h"
#include "PrechargeTask.h" // for hprecharge_task handle

#define FAULT_LOOP_PRINTF_DELAY_MS 10000
#define FAULT_LOOP_PERIOD_MS 500

#define FAULT_PRINTF_COUNTER (FAULT_LOOP_PRINTF_DELAY_MS/FAULT_LOOP_PERIOD_MS)

EventBits_t fault_bits = 0;

StaticTask_t FaultHandler_Task_Buffer;
StackType_t FaultHandler_Task_Stack[PRECHARGE_TASK_STACK_SIZE];

void Init_FaultHandlerTask()
{   
    if (faultHandler_init() != 1)
    {
        // Fault bit initialization failed
        Error_Handler();
    }
}

void Kill_Precharge_Task()
{   
    if (hprecharge_task != NULL)
    {
        vTaskDelete(hprecharge_task);
    }
    
}

static void print_fault(){
    switch (fault_bits) // compare against individual bitmasks
        {
            case FAULT_BIT(ARRAY_GREATER_THAN_BATTERY_FAULT):
                printf("Fault: ARRAY Voltage Greater Than Battery Voltage\r\n");
                break;
            case FAULT_BIT(BATTERY_OVERVOLTAGE_FAULT):
                printf("Fault: Battery Overvoltage\r\n");
                break;
            case FAULT_BIT(BATTERY_UNDERVOLTAGE_FAULT):
                printf("Fault: Battery Undervoltage\r\n");
                break;
            case FAULT_BIT(CONTACTOR_TIMEOUT_FAULT):
                printf("Fault: Contactor Sense Timeout\r\n");
                break;
            case FAULT_BIT(PRECHARGE_TIMEOUT_FAULT):
                printf("Fault: Precharge Sequence Timeout\r\n");
                break;
            case FAULT_BIT(CONTACTOR_UNEXPECTED_STATE_FAULT):
                printf("Fault: Contactor Unexpected State Fault\r\n");
                break;
            default:
                printf("Fault: Unknown\r\n");
                break;
        }
}

void Fault_Loop()
{
    bool toggle = true;
    uint32_t fault_printf_debug_counter = 0;
    while (1)
    {
        fault_printf_debug_counter++;

        if(fault_printf_debug_counter >= FAULT_PRINTF_COUNTER){
            print_fault();
            fault_printf_debug_counter = 0;
        }

        setHeartbeat(toggle);
        toggle = !toggle;
        vTaskDelay(pdMS_TO_TICKS(FAULT_LOOP_PERIOD_MS));

    }
}

void Set_Fault_LED()
{
    switch (fault_bits) // compare against individual bitmasks
    {
    case FAULT_BIT(BATTERY_OVERVOLTAGE_FAULT):
        LED_set(OVER_V_LED, ON);
        break;
    case FAULT_BIT(BATTERY_UNDERVOLTAGE_FAULT):
        LED_set(LOW_V_LED, ON);
        break;
    default:
        break;
    }
}

void Task_FaultHandler()
{   
    LEDs_init();

    Init_FaultHandlerTask();
    
    while (true) {
    LEDsModFaultBitmap_set(0x1F);

    fault_bits = faultBit_wait(NUM_FAULTS, portMAX_DELAY);

    if (fault_bits != 0)
    {   
        LEDs_clear();
        LED_set(FAULT_LED, ON);

        Kill_Precharge_Task();
        emergency_open_contactors();

        print_fault();

        Fault_Loop();
        
    }
    
    vTaskDelay(pdMS_TO_TICKS(500));
    }
}