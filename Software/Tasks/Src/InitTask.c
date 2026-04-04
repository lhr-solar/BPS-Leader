#include "common.h"
#include "BPS_Tasks.h"
#include "EMC2305_Driver.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "Contactors.h"
#include "DebugPrintf.h"
#include "SHT45.h"

// Task Stack Arrays
StackType_t Task_Temperature_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
StackType_t FaultHandler_Task_Stack[ FAULT_HANDLER_TASK_STACK_SIZE ];
StackType_t Precharge_Task_Stack[ PRECHARGE_TASK_STACK_SIZE ];
StackType_t Task_Voltage_Stack_Array[ TASK_VOLTAGE_MONITOR_STACK_SIZE ];
StackType_t Task_Amperes_Stack_Array[ TASK_AMPERES_MONITOR_STACK_SIZE ];
StackType_t Task_Petwdog_Stack_Array[ TASK_PETWDOG_STACK_SIZE ];
StackType_t Task_Contactor_Monitoring_Stack[ TASK_CONTACTOR_MONITORING_STACK_SIZE ];
StackType_t Init_Task_Stack[ FAULT_HANDLER_TASK_STACK_SIZE ];
StackType_t Task_Can_Forward_Stack[ TASK_CAN_FORWARD_STACK_SIZE ];

// Task Buffers
StaticTask_t Init_Task_Buffer;
StaticTask_t FaultHandler_Task_Buffer;
StaticTask_t Task_Temperature_Buffer;
StaticTask_t Task_Voltage_Buffer;
StaticTask_t Task_Amperes_Buffer;
StaticTask_t Task_Petwdog_Buffer;
StaticTask_t Precharge_Task_Buffer;
StaticTask_t Task_Contactor_Monitoring_Buffer;
StaticTask_t Task_Can_Forward_Buffer;

// Event Group
EventGroupHandle_t xWDogEventGroup_handle;

void Task_Init(){

    CAN_Init();
    
    LEDs_init();

    contactor_init();

    SHT45_init();

    EMC2305_Driver_init();

    debugPrintf_init();

    printf("Initialized\n\r");

    // Init_WDogTask();

    xTaskCreateStatic(
        Task_FaultHandler,             // Task function
        "FaultHandler",                // Name of the task (for debugging)
        FAULT_HANDLER_TASK_STACK_SIZE,   // Stack size in words
        (void*)NULL,                       // Task input parameter
        TASK_FAULT_HANDLER_PRIO,       // Task priority
        FaultHandler_Task_Stack,       // Task handle
        &FaultHandler_Task_Buffer      // Static task buffer (optional)
    );
    

    
    //xTaskCreateStatic(
    //    Task_Temperature_Monitor,           /* The function that implements the task. */
    //    "Temperature Monitor Task",         /* Text name for the task. */
    //    TASK_TEMPERATURE_MONITOR_STACK_SIZE,/* The size (in words) of the stack that should be created for the task. */
    //    (void*)NULL,                        /* Paramter passed into the task. */
    //    TASK_TEMPERATURE_MONITOR_PRIO,      /* Task Prioriy. */
    //    Task_Temperature_Stack_Array,       /* Stack array. */
    //    &Task_Temperature_Buffer            /* Buffer for static allocation. */
    // );

   // WIP
//    xTaskCreateStatic(
//        Task_Voltage_Monitor,               /* The function that implements the task. */
//        "Voltage Monitor Task",             /* Text name for the task. */
//        TASK_VOLTAGE_MONITOR_STACK_SIZE,    /* The size (in words) of the stack that should be created for the task. */
//        (void*)NULL,                        /* Paramter passed into the task. */
//        TASK_VOLTAGE_MONITOR_PRIO,          /* Task Prioriy. */
//        Task_Voltage_Stack_Array,           /* Stack array. */
//        &Task_Voltage_Buffer                /* Buffer for static allocation. */
//   );
   

    // xTaskCreateStatic(
    //     Task_PetWatchdog,                   /* The function that implements the task. */
    //     "PetWatchdog",                      /* Text name for the task. */
    //     TASK_PETWDOG_STACK_SIZE,            /* The size (in words) of the stack that should be created for the task. */
    //     (void*)NULL,                        /* Paramter passed into the task. */
    //     TASK_PETWDOG_PRIO,                  /* Task Prioriy. */
    //     Task_Petwdog_Stack_Array,           /* Stack array. */
    //     &Task_Petwdog_Buffer                /* Buffer for static allocation. */
    // );


   // Task deletes itself after all other taks are init'd
    vTaskDelete(NULL);
}