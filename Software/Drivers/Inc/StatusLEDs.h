#include "common.h"

#define MOD_FAULT_BITS 5

typedef enum Fault_Mapping_t {

    HEARTBEAT_LED,
    FAULT_LED,
    OVER_V_LED,
    LOW_V_LED,
    OVER_AMP_LED,
    OVER_TEMP_LED,
    CHARGING_LED,
    VTEMP_IN_LED,
    AMP_IN_LED,
    WATCHDOG_ERR__LED,
    FAULT_LED_NUM,
    DEBUG_LED = 15

} Fault_Mapping_t;

void LED_set(Fault_Mapping_t LED, bool state);

void LEDsModFaultBitmap_set(uint8_t bitmap);

void LEDs_clear(void);

void LEDs_init(void);


