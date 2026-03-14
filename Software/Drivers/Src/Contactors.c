
#include "Contactors.h"

static SemaphoreHandle_t contactorsMutex = NULL;
static StaticSemaphore_t contactorsMutexBuffer;

static const char* CONTACTOR_NAMES[NUM_CONTACTORS] = {
    "HV Positive Contactor",
    "HV Negative Contactor",
    "Array Contactor",
    "Array Precharge Contactor"
};

// array to hold the contactor structs
static contactor_t contactors[NUM_CONTACTORS];

// get physical state
contactor_state_t contactor_get(contactor_num_t contactor_num) {
    
    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS)) {
        Error_Handler();
    }

    contactor_t* contactor = &contactors[contactor_num];
    return HAL_GPIO_ReadPin(contactor->sense_pin.port, contactor->sense_pin.pin);
}

// get expected state
contactor_state_t contactor_get_command_state(contactor_num_t contactor_num) {
    
    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS)) {
        Error_Handler();
    }

    contactor_t* contactor = &contactors[contactor_num];
    return contactor->state;
}

static void vContactorCallback( TimerHandle_t senseTimer ) {

    contactor_num_t contactor_num = (contactor_num_t)pvTimerGetTimerID(senseTimer);
    contactor_t* contactor = &contactors[contactor_num];

    if (contactor->state != contactor_get(contactor_num)) {
        set_faultBit(CONTACTOR_UNEXPECTED_STATE_FAULT);
    }
}

// sets contactor, updates state value, then starts timer to check expected state matches actual state. 
ErrorStatus contactor_set(contactor_num_t contactor_num, contactor_state_t state, uint32_t wait_ms, fault_state_t fault_state) {
    
    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS)) {
        Error_Handler();
    }

    contactor_t* contactor = &contactors[contactor_num];

    // if its emergency, dont bother with semaphore
    if ((fault_state != EMERGENCY) && xSemaphoreTake(contactorsMutex, wait_ms) == pdFALSE) {
        return ERROR;
    };

    // critical section:
    HAL_GPIO_WritePin(contactor->control_pin.port, contactor->control_pin.pin, state);
    contactor->state = state;

    /* start timer to check if the state of the contactor makes expected state, the exit critical section. Timer resets
    when the contactor is set to another value, so no possible error with expected value changing from when timer is called*/
    if ((fault_state != EMERGENCY)) { 
        xTimerStart(contactor->senseTimer, 0); 
        xSemaphoreGive(contactorsMutex);
    }

    return SUCCESS;
}

void contactor_init() {

    // Enable clock for GPIO A/B/C 
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // INITIALIZE MUTEX
    contactorsMutex = xSemaphoreCreateMutexStatic(&contactorsMutexBuffer);

    // initializing the pindef into contactor structs
    contactors[HV_PLUS_CONTACTOR] = (contactor_t){   
        .control_pin = { HV_PLUS_CONTROL_PORT, HV_PLUS_CONTROL_PIN },
        .sense_pin  = { HV_PLUS_SENSE_PORT,  HV_PLUS_SENSE_PIN  }
    };
    
    contactors[HV_MINUS_CONTACTOR] = (contactor_t){   
        .control_pin = { HV_MINUS_CONTROL_PORT, HV_MINUS_CONTROL_PIN },
        .sense_pin  = { HV_MINUS_SENSE_PORT,  HV_MINUS_SENSE_PIN  }
    };
    
    contactors[ARRAY_CONTACTOR] = (contactor_t){
        .control_pin = { ARRAY_CONTROL_PORT, ARRAY_CONTROL_PIN },
        .sense_pin  = { ARRAY_SENSE_PORT,  ARRAY_SENSE_PIN  }
    };

    contactors[ARRAY_PRE_CONTACTOR] = (contactor_t){
        .control_pin = { ARRAY_PRE_CONTROL_PORT, ARRAY_PRE_CONTROL_PIN },
        .sense_pin  = { ARRAY_PRE_SENSE_PORT,  ARRAY_PRE_SENSE_PIN  }
    };
    
    
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    // loop to intialize contactor GPIO and timers
    for (uint32_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++) {

        contactor_t* contactor = &contactors[contactor_num];

        // init contactor control pins
        GPIO_InitStruct.Pin = contactor->control_pin.pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(contactor->control_pin.port, &GPIO_InitStruct);

        // init contactor sense pins 
        GPIO_InitStruct.Pin = contactor->sense_pin.pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(contactor->sense_pin.port, &GPIO_InitStruct);

        contactor->state = contactor_get(contactor_num);

        // making timers and putting them into contactor structs
        contactor->senseTimer = xTimerCreateStatic(
            CONTACTOR_NAMES[contactor_num],                     /* Name of the timer */
            CONTACTOR_SENSE_DELAY,                              /* Timer period in ticks */
            pdFALSE,                                            /* Don't auto-reload */
            (void*)contactor_num,                               /* Timer ID */
            vContactorCallback,                                 /* Callback function */
            &(contactor->senseTimerBuffer)       /* Buffer to hold timer data */
        );
    }
}

