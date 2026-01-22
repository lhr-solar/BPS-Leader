#include "common.h"

#define CONTACTOR_SENSE_DELAY pdMS_TO_TICKS(1000)
#define CALLBACK_BLOCKING_TIME pdMS_TO_TICKS(20)

#define NUM_CONTACTORS 4

typedef struct {
    GPIO_TypeDef* port; // e.g., GPIOA
    uint16_t      pin_num;  // e.g., GPIO_PIN_3
} GpioPin_t;

typedef struct contactor_t {

    bool state;
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


// sets contactor to a value, then calls callback funcion to make sure its set right (unless it is an emergency)
ErrorStatus setContactor(contactor_enum_t contactor_num, bool state, bool blocking, bool emergency);

// called when the sense delay timer ends, makes sure contactor physical state matches what the code thinks it is
void vContactorCallback( TimerHandle_t senseTimer ); 

// gets physical state of a contactor
bool Contactors_Get(contactor_enum_t contactor_num);

// initializes contactor pins from pindef, their respective timers, intializes mutex
void init_contactors();




