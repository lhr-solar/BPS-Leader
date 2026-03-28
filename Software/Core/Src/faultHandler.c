#include "faultHandler.h"
#include "Contactors.h"
#include "EMC2305_Driver.h"
#include "StatusLEDs.h"

// Event group handle to store fault state bits
EventGroupHandle_t faultBits;

// Static buffer to store the event handle
StaticEventGroup_t faultBitsBuffer;

uint8_t faultHandler_init(void){
    faultBits = xEventGroupCreateStatic( &faultBitsBuffer );
    if(faultBits == NULL){
        return 0;
    }
    return 1;
}

void set_faultBit(fault_bit_t bit){
    // not   valid fault
    if(bit >= NUM_FAULTS){ 
        return;
    }

    // chat we're cooked
    xEventGroupSetBits(faultBits, FAULT_BIT(bit));
    // should never return from here
    taskYIELD();
}

void set_faultBitFromISR(fault_bit_t bit, BaseType_t* xHigherPriorityTaskWoken){

    if(bit >= NUM_FAULTS){
        return;
    }

    xEventGroupSetBitsFromISR(
        faultBits,
        FAULT_BIT(bit),
        xHigherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

EventBits_t faultBit_wait(fault_bit_t bit, TickType_t xTicksToWait){

    // NUM_FAULTS indiciates you want to wait for all bits
    if(bit > NUM_FAULTS){
        return 0;
    }

    // EventBits_t uxBitsToWaitFor = bit == NUM_FAULTS ?     ALL_FAULT_BITS : (FAULT_BIT(bit));
    EventBits_t uxBitsToWaitFor = (bit == NUM_FAULTS ?       ALL_FAULT_BITS : (FAULT_BIT(bit)));

    EventBits_t pending = xEventGroupWaitBits(
        faultBits,
        uxBitsToWaitFor,  // wait for any defined fault
        pdFALSE,          // fault bits are not reset
        pdFALSE,          // wait for ANY bit to be set
        xTicksToWait 
    );
    return pending;
}