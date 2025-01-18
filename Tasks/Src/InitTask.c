#include "BPS_Tasks.h"

// Task Stack Arrays
StackType_t Task_Init_Stack_Array[ configMINIMAL_STACK_SIZE ];
StackType_t Task_Temperature_Stack_Array[ configMINIMAL_STACK_SIZE ];
StackType_t Task_Voltage_Stack_Array[ configMINIMAL_STACK_SIZE ];
StackType_t Task_Amperes_Stack_Array[ configMINIMAL_STACK_SIZE ];
StackType_t Task_Petwdog_Stack_Array[ configMINIMAL_STACK_SIZE ];

// // Task Buffers
StaticTask_t Task_Init_Buffer;
StaticTask_t Task_Temperature_Buffer;
StaticTask_t Task_Voltage_Buffer;
StaticTask_t Task_Amperes_Buffer;
StaticTask_t Task_Petwdog_Buffer;

void Task_Init(){
    // Task deletes itself when all other tasks are Init'd
}