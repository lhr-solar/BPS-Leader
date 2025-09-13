#include "BPS_Tasks.h"
#include "LEDs.h"

// Task Stack Arrays
StackType_t Task_Temperature_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
StackType_t Task_Voltage_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
StackType_t Task_Amperes_Stack_Array[ TASK_AMPERES_MONITOR_STACK_SIZE ];
StackType_t Task_Petwdog_Stack_Array[ TASK_PETWDOG_STACK_SIZE ];

// Task Buffers
StaticTask_t Task_Temperature_Buffer;
StaticTask_t Task_Voltage_Buffer;
StaticTask_t Task_Amperes_Buffer;
StaticTask_t Task_Petwdog_Buffer;

// Event Group
EventGroupHandle_t xWDogEventGroup_handle;


void Task_Init(){
    Init_WDogTask();
    Heartbeat_Init();

    xTaskCreateStatic(
        Task_Temperature_Monitor,           /* The function that implements the task. */
        "Temperature Monitor Task",         /* Text name for the task. */
        TASK_TEMPERATURE_MONITOR_STACK_SIZE,/* The size (in words) of the stack that should be created for the task. */
        (void*)NULL,                        /* Paramter passed into the task. */
        TASK_TEMPERATURE_MONITOR_PRIO,      /* Task Prioriy. */
        Task_Temperature_Stack_Array,       /* Stack array. */
        &Task_Temperature_Buffer            /* Buffer for static allocation. */
   );

    xTaskCreateStatic(
        Task_Voltage_Monitor,               /* The function that implements the task. */
        "Voltage Monitor Task",             /* Text name for the task. */
        TASK_VOLTAGE_MONITOR_STACK_SIZE,    /* The size (in words) of the stack that should be created for the task. */
        (void*)NULL,                        /* Paramter passed into the task. */
        TASK_VOLTAGE_MONITOR_PRIO,          /* Task Prioriy. */
        Task_Voltage_Stack_Array,           /* Stack array. */
        &Task_Voltage_Buffer                /* Buffer for static allocation. */
   );

   xTaskCreateStatic(
        Task_PetWatchdog,                   /* The function that implements the task. */
        "PetWatchdog",                      /* Text name for the task. */
        TASK_PETWDOG_STACK_SIZE,            /* The size (in words) of the stack that should be created for the task. */
        (void*)NULL,                        /* Paramter passed into the task. */
        TASK_PETWDOG_PRIO,                  /* Task Prioriy. */
        Task_Petwdog_Stack_Array,           /* Stack array. */
        &Task_Petwdog_Buffer                /* Buffer for static allocation. */
   );


   // Task deletes itself after all other taks are init'd
    vTaskDelete(NULL);
}