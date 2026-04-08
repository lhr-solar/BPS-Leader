#pragma once

#include "common.h"
#include "BPSCAN_can_msgs.h"
#include <event_groups.h>

// Task configuration
#define TASK_INIT_PRIO                  tskIDLE_PRIORITY + 2
#define TASK_TEMPERATURE_MONITOR_PRIO   tskIDLE_PRIORITY + 3
#define TASK_VOLTAGE_MONITOR_PRIO       tskIDLE_PRIORITY + 3
#define TASK_AMPERES_MONITOR_PRIO       tskIDLE_PRIORITY + 3
#define TASK_PETWDOG_PRIO               tskIDLE_PRIORITY + 1
#define TASK_CAN_FORWARD_PRIO           tskIDLE_PRIORITY + 2
#define TASK_FAULT_HANDLER_PRIO         tskIDLE_PRIORITY + 4
#define TASK_CONTACTOR_MONITOR_PRIO     tskIDLE_PRIORITY + 3

#define TEST_TASK_PRIORITY              tskIDLE_PRIORITY + 2

// Task Stack Size 
#define TASK_INIT_STACK_SIZE                     (configMINIMAL_STACK_SIZE*2)
#define TASK_TEMPERATURE_MONITOR_STACK_SIZE      (configMINIMAL_STACK_SIZE*2)
#define TASK_VOLTAGE_MONITOR_STACK_SIZE          (configMINIMAL_STACK_SIZE*2)
#define TASK_AMPERES_MONITOR_STACK_SIZE          (configMINIMAL_STACK_SIZE*2)
#define TASK_PETWDOG_STACK_SIZE                  (configMINIMAL_STACK_SIZE*2)
#define PRECHARGE_TASK_STACK_SIZE                (configMINIMAL_STACK_SIZE*2)
#define FAULT_HANDLER_TASK_STACK_SIZE            (configMINIMAL_STACK_SIZE*2)
#define TASK_CAN_FORWARD_STACK_SIZE              (configMINIMAL_STACK_SIZE*2)
#define TASK_CONTACTOR_MONITORING_STACK_SIZE     (configMINIMAL_STACK_SIZE*2)

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

// Task Delays
#define TEMP_MONITOR_TASK_DELAY_MS      290
#define VOLT_MONITOR_TASK_DELAY_MS      290
#define PRECHARGE_TASK_DELAY_MS         100
#define CONTACTOR_MONITOR_TASK_DELAY_MS 50
#define AMPERES_MONITOR_TASK_DELAY_MS   90

// Task Inits
void Task_Init();
void Task_Voltage_Monitor();
void Task_Temperature_Monitor();
void Task_FaultHandler(void *pvParameters);
void Task_Temperature_Monitor();
void Task_Amperes_Monitor();
void Task_PetWatchdog();
void Task_Temperature_Monitor();
void Task_CanRxForward();
void Task_Contactor_Monitor();


extern bps_pack_current_t AmperesData;

/* ---- Watchdog Event Group ---- */
void Init_WDogTask();
extern EventGroupHandle_t xWDogEventGroup_handle;
#define TEMP_MONITOR_DONE   (1 << 0)
#define VOLT_MONITOR_DONE   (1 << 1)
#define WINDOW_TIMER_DONE   (1 << 2)
#define AMPERES_MONITOR_DONE (1 << 3)
#define ALL_TASKS_DONE (TEMP_MONITOR_DONE | VOLT_MONITOR_DONE | WINDOW_TIMER_DONE | AMPERES_MONITOR_DONE)

// Task Checkin init stuff.
extern EventGroupHandle_t xTaskBits;
extern EventGroupHandle_t xStateBits;

#define set_task_bit(bit) (xEventGroupSetBits(xTaskBits, (1U << bit))) 
#define set_state_bit(bit) (xEventGroupSetBits(xStateBits, (1U << bit))) 

#define get_state_bit(bit) ((xEventGroupGetBits(xStateBits) & (1U << bit)) >> bit)
#define get_task_bit(bit) ((xEventGroupGetBits(xTaskBits) & (1U << bit)) >> bit)

#define clear_state_bit(bit) (xEventGroupClearBits(xStateBits, (1U << bit)))

typedef enum {
    AMPHERES_MONITOR,
    VOLTAGE_MONITOR,
    TEMPERATURE_MONITOR,
    CONTACTOR_MONITOR,
    NUM_TASK_CHECK_BITS
} task_check_bits;

typedef enum {
    DISCHARGING_BATT_STATE,    // 1 = Discharging, 0 = charging
} state_bits;



