#include "FaultHandlerTask.h"
#include "PrechargeTask.h" // for hprecharge_task handle

#define FAULT_LOOP_PRINTF_DELAY_MS 1000
#define FAULT_LOOP_PERIOD_MS 500

#define FAULT_PRINTF_COUNTER (FAULT_LOOP_PRINTF_DELAY_MS / FAULT_LOOP_PERIOD_MS)

EventBits_t fault_bits = 0;

void Init_FaultHandlerTask() {
    if (faultHandler_init() != 1)
    {
        // Fault bit initialization failed
        Error_Handler();
    }

    fault_task_initialized = true;
}

void Kill_Precharge_Task() {
    if (hprecharge_task != NULL)
    {
        vTaskDelete(hprecharge_task);
    }
}

// Print faults & set relevant LEDs
static void print_fault() {
    printf("================================\r\n");

    switch (fault_bits) // compare against individual bitmasks
    {
        // BPS Board Faults (VoltTemp, Amperes, Leader)
    case FAULT_BIT(CELL_OVERVOLTAGE_FAULT):
        LED_set(OVER_V_LED, LED_ON);
        printf("FAULT: CELL OVERVOLTAGE\r\n");
        break;
    case FAULT_BIT(CELL_UNDERVOLTAGE_FAULT):
        LED_set(LOW_V_LED, LED_ON);
        printf("FAULT: CELL UNDERVOLTAGE\r\n");
        break;
    case FAULT_BIT(BQ_CHIP_FAULT):
        printf("FAULT: BQ CHIP ERR\r\n");
        break;
    case FAULT_BIT(CELL_OVERTEMP_FAULT):
        LED_set(OVER_TEMP_LED, LED_ON);
        printf("FAULT: CELL OVERTEMP\r\n");
        break;
    case FAULT_BIT(PACK_OVERCURRENT_CHARGING_FAULT):
        LED_set(OVER_AMP_LED, LED_ON);
        printf("FAULT: PACK OVERCURRENT WHILE CHARGING\r\n");
        break;
    case FAULT_BIT(PACK_OVERCURRENT_DISCHARGING_FAULT):
        LED_set(OVER_AMP_LED, LED_ON);
        printf("FAULT: PACK OVERCURRENT WHILE DISCHARGING\r\n");
        break;
    // erm will fix after charging
    // case FAULT_BIT(BOARD_OVERTEMP_FAULT):
    //     printf("FAULT: BPS COMPARTMENT OVERTEMP\r\n");
    //     break;
    case FAULT_BIT(BPS_CAN_ERROR):
        printf("FAULT: BPS CAN ERR\r\n");
        break;


        // Contactor Faults
    case FAULT_BIT(CONTACTOR_HV_PLUS_FAULT):
        printf("FAULT: HV PLUS CONTACTOR\r\n");
        break;
    case FAULT_BIT(CONTACTOR_HV_MINUS_FAULT):
        printf("FAULT: HV MINUS CONTACTOR\r\n");
        break;
    case FAULT_BIT(CONTACTOR_ARRAY_FAULT):
        printf("FAULT: ARRAY CONTACTOR\r\n");
        break;
    case FAULT_BIT(CONTACTOR_ARRAY_PRE_FAULT):
        printf("FAULT: ARRAY PRECHARGE CONTACTOR\r\n");
        break;
    case FAULT_BIT(CONTACTOR_CALLBACK_FAULT):
        printf("FAULT: CONTACTOR CALLBACK\r\n");
        break;

        // Precharge Faults
    case FAULT_BIT(ARRAY_GREATER_THAN_BATTERY_FAULT):
        printf("FAULT: ARRAY > BATTERY VOLTAGE\r\n");
        break;
    case FAULT_BIT(PRECHARGE_TIMEOUT_FAULT):
        printf("FAULT: PRECHARGE TIMEOUT\r\n");
        break;
    case FAULT_BIT(PRECHARGE_HYSTERESIS_FAULT):
        printf("FAULT: PRECHARGE HYSTERESIS\r\n");
        break;

        // E-stop Faults
    case FAULT_BIT(BPS_ESTOP1_FAULT):
        printf("FAULT: ESTOP 1\r\n");
        break;
    case FAULT_BIT(BPS_ESTOP2_FAULT):
        printf("FAULT: ESTOP 2\r\n");
        break;
    case FAULT_BIT(BPS_ESTOP3_FAULT):
        printf("FAULT: ESTOP 3\r\n");
        break;
    default:
        printf("FAULT: UNHANDLED\r\n");
        break;

        // Software Faults
    case FAULT_BIT(RTOS_WATCHDOG_ERROR):
        printf("FAULT: RTOS WATCHDOG\r\n");
        break;
    case FAULT_BIT(CAR_CAN_ERROR):
        printf("FAULT: CAR CAN ERR\r\n");
        break;
    // case FAULT_BIT(I2C_ERROR):
    //     printf("FAULT: I2C ERR\r\n");
    //     break;
    // case FAULT_BIT(ADC_ERROR):
    //     printf("FAULT: ADC ERR\r\n");
    //     break;
    case FAULT_BIT(FAN_CHIP_ERROR):
        printf("FAULT: FAN CHIP ERR\r\n");
        break;
    }

    printf("================================\r\n");
}

void Fault_Loop() {

    uint32_t fault_printf_debug_counter = 0;
    while (1)
    {
        fault_printf_debug_counter++;

        if (fault_printf_debug_counter >= FAULT_PRINTF_COUNTER)
        {
            print_fault();
            fault_printf_debug_counter = 0;
        }

        toggleHeartbeat();
        vTaskDelay(pdMS_TO_TICKS(FAULT_LOOP_PERIOD_MS));
    }
}

void Task_FaultHandler(void* pvParameters) {
    Init_FaultHandlerTask();

    while (true)
    {
        // Wait indefintiely for any fault bit to be set
        fault_bits = faultBit_wait(NUM_FAULTS, portMAX_DELAY);

        if (fault_bits != 0)
        {
            LEDs_clear();
            LED_set(FAULT_LED, LED_ON);

            Kill_Precharge_Task();
            emergency_open_contactors();

            print_fault();

            Fault_Loop(); // WILL NEVER RETURN - while(true)
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
