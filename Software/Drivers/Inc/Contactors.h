#include "common.h"

#define CONTACTOR_SENSE_DELAY pdMS_TO_TICKS(1000)
#define CALLBACK_BLOCKING_TIME pdMS_TO_TICKS(20)

#define NUM_CONTACTORS 4
#define CONTACTOR_INVALID -1

typedef struct contactor_t {

    volatile bool state;
    bool expected_state;
    GpioPin_t sense_pin;
    GpioPin_t control_pin;
    TimerHandle_t senseTimer; // timer handle for checking if contactor closed/opened (non-blocking mode)
    StaticTimer_t senseTimerBuffer;
    
} contactor_t;

typedef enum contactor_enum_t {
    HV_Pos_contactor = 0,
    HV_Neg_contactor,
    Array_contactor,
    Array_Pre_contactor,
} contactor_enum_t;


// initializes contactor pins from pindef, their respective timers, intializes mutex
void init_contactors();

// redefine weakly defined HAL GPIO pin toggle interrupt, updates state values when sense pins are toggled
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

// gets physical state of a contactor
bool Contactors_Get(contactor_enum_t contactor_num);

// sets contactor to a value, then calls callback funcion to make sure its set right (unless it is an emergency)
ErrorStatus setContactor(contactor_enum_t contactor_num, bool state, bool blocking, bool emergency);

// called when the sense delay timer ends, makes sure contactor physical state matches what the code thinks it is
void vContactorCallback( TimerHandle_t senseTimer ); 




