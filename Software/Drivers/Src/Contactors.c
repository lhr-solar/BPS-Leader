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

void init_contactors() {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    // Enable clock for GPIO A/B/C 
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // init channel A contactor pins 
    GPIO_InitStruct.Pin = HV_POSITIVE_SENSE_PIN_NUM | HV_NEGATIVE_SENSE_PIN_NUM | ARRAY_SENSE_PIN_NUM;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // init channel B contactor pins
    GPIO_InitStruct.Pin = HV_NEGATIVE_CONTROL_PIN_NUM | ARRAY_CONTROL_PIN_NUM | ARRAY_PRE_CONTROL_PIN_NUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // init channel C contactor pins
    GPIO_InitStruct.Pin = HV_POSITIVE_CONTROL_PIN_NUM; 
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // initializing the pindef into contactor structs
    contactors[HV_Pos_contactor] = (contactor_t){   
        .control_pin = { HV_POSITIVE_CONTROL_PORT, HV_POSITIVE_CONTROL_PIN_NUM },
        .sense_pin  = { HV_POSITIVE_SENSE_PORT,  HV_POSITIVE_SENSE_PIN_NUM  }
    };

    contactors[HV_Neg_contactor] = (contactor_t){   
        .control_pin = { HV_NEGATIVE_CONTROL_PORT, HV_NEGATIVE_CONTROL_PIN_NUM },
        .sense_pin  = { HV_NEGATIVE_SENSE_PORT,  HV_NEGATIVE_SENSE_PIN_NUM  }
    };
    
    contactors[Array_contactor] = (contactor_t){
        .control_pin = { ARRAY_CONTROL_PORT, ARRAY_CONTROL_PIN_NUM },
        .sense_pin  = { ARRAY_SENSE_PORT,  ARRAY_SENSE_PIN_NUM  }
    };

    contactors[Array_Pre_contactor] = (contactor_t){
        .control_pin = { ARRAY_PRE_CONTROL_PORT, ARRAY_PRE_CONTROL_PIN_NUM },
        .sense_pin  = { ARRAY_PRE_SENSE_PORT,  ARRAY_PRE_SENSE_PIN_NUM  }
    };

    // loop to intialize contactor timers 
    for (uint32_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++) {
        // making timers and putting them into structs
        contactors[contactor_num].senseTimer = xTimerCreateStatic(
            CONTACTOR_NAMES[contactor_num],                     /* Name of the timer */
            CONTACTOR_SENSE_DELAY,                              /* Timer period in ticks */
            pdFALSE,                                            /* Don't auto-reload */
            (void*)contactor_num,                               /* Timer ID */
            vContactorCallback,                                 /* Callback function */
            &(contactors[contactor_num].senseTimerBuffer)       /* Buffer to hold timer data */
        );
    }

    // INITIALIZE MUTEX
    contactorsMutex = xSemaphoreCreateMutexStatic(&contactorsMutexBuffer);
}

bool Contactors_Get(contactor_enum_t contactor_num) {

    contactor_t* contactor = &contactors[contactor_num];
    return HAL_GPIO_ReadPin(contactor->sense_pin.port, contactor->sense_pin.pin_num);

}

void vContactorCallback( TimerHandle_t senseTimer ) {

    contactor_enum_t contactor_num = (contactor_enum_t)pvTimerGetTimerID(senseTimer);
    contactor_t* contactor = &contactors[contactor_num];

    if (contactor->state != Contactors_Get(contactor_num)) {
        // This means the contactor actual state is not what the code thinks it is. call fault handler.
    }

}

ErrorStatus setContactor(contactor_enum_t contactor_num, bool state, bool blocking, bool emergency) {
    
    contactor_t* contactor = &contactors[contactor_num];

    // if its emergency, dont bother with semaphore
    if (!emergency && xSemaphoreTake(contactorsMutex, blocking ? portMAX_DELAY : 0) == pdFALSE) {
        return ERROR;
    };

    // critical section:
    HAL_GPIO_WritePin(contactor->control_pin.port, contactor->control_pin.pin_num, state);
    contactor->state = state;
    
    // exit critical section, return mutex
    if (!emergency && xSemaphoreGive(contactorsMutex) == pdFALSE) {
        return ERROR;
    }

    // start callback function timer, unless its an emergency 
    if (!emergency) { xTimerStart(contactor->senseTimer, 0); }

    return SUCCESS;

}

