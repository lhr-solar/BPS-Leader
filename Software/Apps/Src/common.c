#include "Contactors.h"
#include "StatusLEDs.h"
#include "common.h"

/* void faultHandler() {

    // kill RTOS 
    __disable_irq();
    
    // open every contactor, bypasses semaphore
    for (uint8_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++) {
        // blocking doesn't actually do anything since emergency flag is set
        toggle_contactor(contactor_num, OPEN, BLOCKING, EMERGENCY);
    }
    

    

}
 */