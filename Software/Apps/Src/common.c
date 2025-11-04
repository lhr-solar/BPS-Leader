#include "Contactors.h"
#include "StatusLEDs.h"

void faultHandler(firmware_error_code_t firmware_error_code, uint16_t hardware_fault_bitmap) {

    // kill RTOS 
    __disable_irq();
    
    // open every contactor, bypasses semaphore
    for (uint8_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++) {
        // blocking doesn't actually do anything since emergency flag is set
        toggle_contactor(contactor_num, OPEN, BLOCKING, EMERGENCY);
    }

    // TODO: CAN messages
    
    // setup LED messages
    uint16_t LED_STATUS = 0; 

    // puts error code into the 
    error_code <<= 1;
    LED_STATUS |= error_code;

}