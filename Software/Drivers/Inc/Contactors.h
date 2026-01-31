#include "common.h"

#define CONTACTOR_SENSE_DELAY pdMS_TO_TICKS(1000)
#define CALLBACK_BLOCKING_TIME pdMS_TO_TICKS(20)


#define CONTACTOR_INVALID -1

typedef struct contactor_t {
    bool state;
    GpioPin_t sense_pin;
    GpioPin_t control_pin;
    TimerHandle_t senseTimer; // timer handle for checking if contactor closed/opened (non-blocking mode)
    StaticTimer_t senseTimerBuffer;
} contactor_t;

typedef enum contactor_enum_t {
    HV_PLUS_CONTACTOR = 0,
    HV_MINUS_CONTACTOR,
    ARRAY_CONTACTOR,
    ARRAY_PRE_CONTACTOR,
    NUM_CONTACTORS
} contactor_enum_t;


// initializes contactor pins from pindef, their respective timers, intializes mutex
void contactor_init();

// gets physical state of a contactor
bool contactor_get(contactor_enum_t contactor_num);

// sets contactor to a value, then calls callback funcion to make sure its set right (unless it is an emergency)
ErrorStatus contactor_set(contactor_enum_t contactor_num, bool state, bool blocking, bool emergency);



