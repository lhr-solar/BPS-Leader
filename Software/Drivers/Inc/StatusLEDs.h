#include "common.h"

#define LED_NUM 16
#define MOD_FAULT_NUM 5

extern uint8_t modFaultBitmap; 
extern bool debug_LED;

typedef enum Fault_Mapping_t {

    HEARTBEAT_LED = 1 << 0,
    FAULT_LED = 1 << 1,
    OVER_V_LED = 1 << 2,
    LOW_V_LED = 1 << 3,
    OVER_AMP_LED = 1 << 4,
    OVER_TEMP_LED = 1 << 5,
    CHARGING_LED = 1 << 6,
    VTEMP_IN_LED = 1 << 7,
    AMP_IN_LED = 1 << 8,
    WATCHDOG_ERR__LED = 1 << 9,

} Fault_Mapping_t;

void setLED(Fault_Mapping_t LED, bool state);

void loadBit(bool bit);

void pushLEDS();

void init_StatusLEDs();


