#include "faultHandler.h"
#include "Contactors.h"
#include "EMC2305_Driver.h"     
#include "StatusLEDs.h"

bool fault_task_initialized = false;

bool system_has_faulted = false;
volatile uint32_t first_fault_id;
static StaticSemaphore_t xFaultSemaphoreBuffer;

// Event group handles to store fault state bits (split into two fault bit buffers)
EventGroupHandle_t faultBits_1;
EventGroupHandle_t faultBits_2; 

// Static buffer to store the event handle
StaticEventGroup_t faultBitsBuffer_1;
StaticEventGroup_t faultBitsBuffer_2;

SemaphoreHandle_t faultSemaphore = NULL;

uint8_t faultHandler_init(void){

    if (fault_task_initialized) return 1;

    faultBits_1 = xEventGroupCreateStatic( &faultBitsBuffer_1 );
    faultBits_2 = xEventGroupCreateStatic( &faultBitsBuffer_2 );

    faultSemaphore = xSemaphoreCreateBinaryStatic(&xFaultSemaphoreBuffer);

    if(faultBits_1 == NULL || faultBits_2 == NULL){
        return 0;
    }

    fault_task_initialized = true;

    return 1;
}

const char* const fault_bit_strings[NUM_FAULTS] = {
    // BPS main safety loop faults
    [BPS_FAULT]                          = "BPS_FAULT",
    [CELL_OVERVOLTAGE_FAULT]             = "CELL_OVERVOLTAGE_FAULT",
    [CELL_UNDERVOLTAGE_FAULT]            = "CELL_UNDERVOLTAGE_FAULT",
    [BQ_CHIP_FAULT]                      = "BQ_CHIP_FAULT",
    [CELL_OVERTEMP_FAULT]                = "CELL_OVERTEMP_FAULT",
    [PACK_OVERCURRENT_CHARGING_FAULT]    = "PACK_OVERCURRENT_CHARGING_FAULT",
    [PACK_OVERCURRENT_DISCHARGING_FAULT] = "PACK_OVERCURRENT_DISCHARGING_FAULT",
    [CONTACTOR_HV_PLUS_FAULT]            = "CONTACTOR_HV_PLUS_FAULT",
    [CONTACTOR_HV_MINUS_FAULT]           = "CONTACTOR_HV_MINUS_FAULT",
    [CONTACTOR_ARRAY_FAULT]              = "CONTACTOR_ARRAY_FAULT",
    [CONTACTOR_ARRAY_PRE_FAULT]          = "CONTACTOR_ARRAY_PRE_FAULT",

    [CONTACTOR_CALLBACK_FAULT]           = "CONTACTOR_CALLBACK_FAULT",
    [AMPERES_WATCHDOG_FAULT]             = "AMPERES_WATCHDOG_FAULT",
    [BPS_ESTOP1_FAULT]                   = "BPS_ESTOP1_FAULT",
    [BPS_ESTOP2_FAULT]                   = "BPS_ESTOP2_FAULT",
    [BPS_ESTOP3_FAULT]                   = "BPS_ESTOP3_FAULT",

    // Precharge faults
    [ARRAY_GREATER_THAN_BATTERY_FAULT]   = "ARRAY_GREATER_THAN_BATTERY_FAULT",
    [PRECHARGE_TIMEOUT_FAULT]            = "PRECHARGE_TIMEOUT_FAULT",
    [PRECHARGE_OUT_OF_BOUNDS_FAULT]      = "PRECHARGE_OUT_OF_BOUNDS_FAULT",
    [PACK_OVERVOLTAGE_FAULT]             = "PACK_OVERVOLTAGE_FAULT",
    [PACK_UNDERVOLTAGE_FAULT]            = "PACK_UNDERVOLTAGE_FAULT",
    [FAN_TACHOMETER_FAULT]               = "FAN_TACHOMETER_FAULT",

    // Software Errors
    [RTOS_WATCHDOG_ERROR]                = "RTOS_WATCHDOG_ERROR",
    [BPS_CAN_ERROR]                      = "BPS_CAN_ERROR",
    [CAR_CAN_ERROR]                      = "CAR_CAN_ERROR",
    [FAN_CHIP_ERROR]                     = "FAN_CHIP_ERROR",
    [ELCON_FAULT]                        = "ELCON_FAULT",
    [REGEN_FAULT]                        = "REGEN_FAULT",
    [ADC_ERROR]                          = "ADC_ERROR"
};

uint32_t faultBit_wait(fault_bit_t bit, TickType_t xTicksToWait){

    // NUM_FAULTS indiciates you want to wait for all bits
    if(bit > NUM_FAULTS){
        return 0;
    }

    // EventBits_t uxBitsToWaitFor = bit == NUM_FAULTS ?     ALL_FAULT_BITS : (FAULT_BIT(bit));
    if (xSemaphoreTake(faultSemaphore, xTicksToWait) == pdTRUE);

    return first_fault_id;
}