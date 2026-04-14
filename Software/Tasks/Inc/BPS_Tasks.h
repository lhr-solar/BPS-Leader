#pragma once

#include "common.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"
#include <event_groups.h>

// Task configuration
#define TASK_INIT_PRIO                  tskIDLE_PRIORITY + 2
#define TASK_TEMPERATURE_MONITOR_PRIO   tskIDLE_PRIORITY + 3
#define TASK_VOLTAGE_MONITOR_PRIO       tskIDLE_PRIORITY + 3
#define TASK_AMPERES_MONITOR_PRIO       tskIDLE_PRIORITY + 3
#define TASK_PETWDOG_PRIO               tskIDLE_PRIORITY + 1
#define TASK_CAN_FORWARD_PRIO           tskIDLE_PRIORITY + 2
#define TASK_FAULT_HANDLER_PRIO         tskIDLE_PRIORITY + 5
#define TASK_CONTACTOR_MONITOR_PRIO     tskIDLE_PRIORITY + 4
#define TASK_FAN_CONTROLLER_PRIO        tskIDLE_PRIORITY + 3
#define TASK_CAN_STATUS_PRIO            tskIDLE_PRIORITY + 4

#define TEST_TASK_PRIORITY              tskIDLE_PRIORITY + 2

// Task Stack Size 
#define TASK_INIT_STACK_SIZE                     (configMINIMAL_STACK_SIZE*2)
#define TASK_TEMPERATURE_MONITOR_STACK_SIZE      (configMINIMAL_STACK_SIZE*2)
#define TASK_VOLTAGE_MONITOR_STACK_SIZE          (configMINIMAL_STACK_SIZE*2)
#define TASK_AMPERES_MONITOR_STACK_SIZE          (configMINIMAL_STACK_SIZE*2)
#define TASK_PETWDOG_STACK_SIZE                  (configMINIMAL_STACK_SIZE*2)
#define PRECHARGE_TASK_STACK_SIZE                (configMINIMAL_STACK_SIZE*2)
#define FAULT_HANDLER_TASK_STACK_SIZE            (configMINIMAL_STACK_SIZE*2)
#define TASK_CAN_FORWARD_STACK_SIZE              (configMINIMAL_STACK_SIZE*4)
#define TASK_CONTACTOR_MONITORING_STACK_SIZE     (configMINIMAL_STACK_SIZE*2)
#define TASK_FAN_CONTROLLER_STACK_SIZE           (configMINIMAL_STACK_SIZE*2)
#define TASK_CAN_STATUS_STACK_SIZE               (configMINIMAL_STACK_SIZE*2)

#define TEST_TASK_STACK_SIZE                     (configMINIMAL_STACK_SIZE*2)

// (exposed so that tests can init tasks)
// Task Stack Arrays 
extern StackType_t Task_Temperature_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
extern StackType_t Task_Voltage_Stack_Array[ TASK_VOLTAGE_MONITOR_STACK_SIZE ];
extern StackType_t Task_Amperes_Stack_Array[ TASK_AMPERES_MONITOR_STACK_SIZE ];
extern StackType_t Task_Petwdog_Stack_Array[ TASK_PETWDOG_STACK_SIZE ];
extern StackType_t Precharge_Task_Stack[ PRECHARGE_TASK_STACK_SIZE ];
extern StackType_t FaultHandler_Task_Stack[ FAULT_HANDLER_TASK_STACK_SIZE ];
extern StackType_t Task_Can_Forward_Stack[ TASK_CAN_FORWARD_STACK_SIZE ];
extern StackType_t Task_Contactor_Monitoring_Stack[ TASK_CONTACTOR_MONITORING_STACK_SIZE ];
extern StackType_t Init_Task_Stack[ TASK_INIT_STACK_SIZE ];
extern StackType_t Task_Fan_Controller_Stack[ TASK_FAN_CONTROLLER_STACK_SIZE ];
extern StackType_t Task_Can_Status_Stack[ TASK_CAN_STATUS_STACK_SIZE ];

// Task Buffers
extern StaticTask_t Task_Temperature_Buffer;
extern StaticTask_t Task_Voltage_Buffer;
extern StaticTask_t Task_Amperes_Buffer;
extern StaticTask_t Task_Petwdog_Buffer;
extern StaticTask_t Precharge_Task_Buffer;
extern StaticTask_t FaultHandler_Task_Buffer;
extern StaticTask_t Task_Can_Forward_Buffer;
extern StaticTask_t Task_Contactor_Monitoring_Buffer;
extern StaticTask_t Init_Task_Buffer;
extern StaticTask_t Task_Fan_Controller_Buffer;
extern StaticTask_t Task_Can_Status_Buffer;

// Task Delays
#define TEMP_MONITOR_TASK_DELAY_MS      290
#define VOLT_MONITOR_TASK_DELAY_MS      290
#define PRECHARGE_TASK_DELAY_MS         100
#define CONTACTOR_MONITOR_TASK_DELAY_MS 200
#define AMPERES_MONITOR_TASK_DELAY_MS   90
#define FAN_CONTROLLER_TASK_DELAY_MS    300
#define CAN_STATUS_TASK_DELAY_MS        500

// Task Inits
void Task_Init();
void Task_Voltage_Monitor();
void Task_Temperature_Monitor();
void Task_FaultHandler();
void Task_Fan_Controller();
void Task_Amperes_Monitor();
void Task_PetWatchdog();
void Task_CanRxForward();
void Task_Contactor_Monitor();
void Task_Can_Status();

extern bps_voltage_aggregate_arr_t volt_can_data[NUM_VOLTAGE_SENSORS];
extern bps_temperature_aggregate_arr_t temp_can_data[NUM_TEMPERATURE_SENSORS];
extern uint32_t exposed_volt_sensor_bitmap;
extern uint32_t exposed_temp_sensor_bitmap;

/* ---- Watchdog Event Group ---- */
void Init_WDogTask();
extern EventGroupHandle_t xWDogEventGroup_handle;
#define TEMP_MONITOR_DONE   (1 << 0)
#define VOLT_MONITOR_DONE   (1 << 1)
#define WINDOW_TIMER_DONE   (1 << 2)
#define AMPERES_MONITOR_DONE (1 << 3)
#define ALL_TASKS_DONE (TEMP_MONITOR_DONE | VOLT_MONITOR_DONE | WINDOW_TIMER_DONE | AMPERES_MONITOR_DONE)

// Task Checkin init stuff.
extern EventGroupHandle_t xStateBits;

extern StaticEventGroup_t xStateBits_buffer;

extern EventGroupHandle_t faultBits_1;
extern EventGroupHandle_t faultBits_2;

#define get_state_bit(bit) ((xEventGroupGetBits(xStateBits) & (1U << bit)) >> bit)

#define set_state_bit(bit, state) ((state) ? (xEventGroupSetBits(xStateBits, (1U << bit))) : (clear_state_bit(bit))) 

#define clear_state_bit(bit) (xEventGroupClearBits(xStateBits, (1U << bit)))

typedef enum {
    DISCHARGING_BATT_STATE,
    CHARGING_BATT_STATE,
    AMPERES_MONITOR_GOOD,
    VOLTAGE_MONITOR_GOOD,
    TEMPERATURE_MONITOR_GOOD,
    CONTACTOR_MONITOR_GOOD,
    NUM_STATE_BITS
} state_bits_t;

#define STATE_BIT_SET (1)
#define STATE_BIT_RESET (0)


