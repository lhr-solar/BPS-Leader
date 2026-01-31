#include "Contactors.h"

static SemaphoreHandle_t contactorsMutex = NULL;
static StaticSemaphore_t contactorsMutexBuffer;

static const char* CONTACTOR_NAMES[NUM_CONTACTORS] = {
    "HV Pos Contactor",
    "HV Neg Contactor",
    "Array Contactor",
    "Array Pre Contactor"
};

// array to hold the contactor structs
static contactor_t contactors[NUM_CONTACTORS];

// get
bool contactor_get(contactor_enum_t contactor_num) {
    
    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS)) {
        Error_Handler();
    }

    contactor_t* contactor = &contactors[contactor_num];
    return HAL_GPIO_ReadPin(contactor->sense_pin.port, contactor->sense_pin.pin_num);
}

static void vContactorCallback( TimerHandle_t senseTimer ) {

    contactor_enum_t contactor_num = (contactor_enum_t)pvTimerGetTimerID(senseTimer);
    contactor_t* contactor = &contactors[contactor_num];

    if (contactor->state != contactor_get(contactor_num)) {
        faultHandler();
    }
}

/* sets contactor, updates state value, then starts timer to check expected state matches actual state. 
An error means semaphore was busy, or that I set a contactor that didn't exist. */
ErrorStatus contactor_set(contactor_enum_t contactor_num, bool state, bool blocking, bool emergency) {
    
    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS)) {
        Error_Handler();
    }

    contactor_t* contactor = &contactors[contactor_num];

    // if its emergency, dont bother with semaphore
    if (!emergency && xSemaphoreTake(contactorsMutex, blocking ? portMAX_DELAY : 0) == pdFALSE) {
        return ERROR;
    };

    // critical section:
    HAL_GPIO_WritePin(contactor->control_pin.port, contactor->control_pin.pin_num, state);
    contactor->state = state;

    /* start timer to check if the state of the contactor makes expected state, the exit critical section. Timer resets
    when the contactor is set to another value, so no possible error with expected value changing from when timer is called*/
    if (!emergency) { 
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
        .control_pin = { HV_PLUS_CONTROL_PORT, HV_PLUS_CONTROL_PIN_NUM },
        .sense_pin  = { HV_PLUS_SENSE_PORT,  HV_PLUS_SENSE_PIN_NUM  }
    };

    contactors[HV_MINUS_CONTACTOR] = (contactor_t){   
        .control_pin = { HV_MINUS_CONTROL_PORT, HV_MINUS_CONTROL_PIN_NUM },
        .sense_pin  = { HV_MINUS_SENSE_PORT,  HV_MINUS_SENSE_PIN_NUM  }
    };
    
    contactors[ARRAY_CONTACTOR] = (contactor_t){
        .control_pin = { ARRAY_CONTROL_PORT, ARRAY_CONTROL_PIN_NUM },
        .sense_pin  = { ARRAY_SENSE_PORT,  ARRAY_SENSE_PIN_NUM  }
    };

    contactors[ARRAY_PRE_CONTACTOR] = (contactor_t){
        .control_pin = { ARRAY_PRE_CONTROL_PORT, ARRAY_PRE_CONTROL_PIN_NUM },
        .sense_pin  = { ARRAY_PRE_SENSE_PORT,  ARRAY_PRE_SENSE_PIN_NUM  }
    };

    
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    // loop to intialize contactor GPIO and timers
    for (uint32_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++) {

        contactor_t* contactor = &contactors[contactor_num];

        // init contactor control pins
        GPIO_InitStruct.Pin = contactor->control_pin.pin_num;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(contactor->control_pin.port, &GPIO_InitStruct);

        // init contactor sense pins 
        GPIO_InitStruct.Pin = contactor->sense_pin.pin_num;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
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

