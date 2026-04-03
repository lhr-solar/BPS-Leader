
#include "BPS_Tasks.h"
#include "CANbus.h"


// placeholder
void Task_Voltage_Monitor(){

    while(1){
        // Delays 10 ms
        vTaskDelay(VOLT_MONITOR_TASK_DELAY_MS);

        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle,     /* The event group being updated. */
                           VOLT_MONITOR_DONE);         /* The bits being set. */
    }
    
}


// WIP (will finish and uncomment in its own branch)


/*
#define TOTAL_AMPHERE_MODULES 8


void vTaskVoltageMonitor(void *pvParameters) {
    // Initialize local tracking variables
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t u32StatusFlags = 0;
    
    // Placeholder for storing incoming CAN data
    ModuleData_t xCurrentModuleData;

    for (;;) {
        bool bAllModulesResponding = true;

        for (int i = 0; i < TOTAL_AMPHERE_MODULES; i++) {
            // 1. Read CAN messages from specific Volt-Temp boards
            // Placeholder: DRIVER_CAN_ReadModuleVoltage(module_id, timeout)
            if (can_send(i, &xCurrentModuleData, CAN_READ_TIMEOUT_MS) == STATUS_OK) {
                
                // 2. Set fault bits if voltage is out of range
                if (xCurrentModuleData.u16Voltage > MAX_SAFE_VOLTAGE_LIMIT || 
                    xCurrentModuleData.u16Voltage < MIN_SAFE_VOLTAGE_LIMIT) {
                    
                    // Logic to set specific module fault bit
                    SET_MODULE_FAULT_BIT(i, FAULT_VOLTAGE_OUT_OF_RANGE);
                    SET_SYSTEM_CRITICAL_FAULT(SYS_FAULT_OVER_UNDER_VOLT);
                } else {
                    // Clear module-specific fault if range is now healthy
                    CLEAR_MODULE_FAULT_BIT(i, FAULT_VOLTAGE_OUT_OF_RANGE);
                }

            } else {
                // 3. Set fault bits if CAN message is not responding (Timeout)
                bAllModulesResponding = false;
                SET_MODULE_FAULT_BIT(i, FAULT_COMMUNICATION_LOSS);
            }
        }

        // Global check for communication health
        if (!bAllModulesResponding) {
            SET_SYSTEM_CRITICAL_FAULT(SYS_FAULT_CAN_COMM_FAILURE);
        }

        xEventGroupSetBits(xWDogEventGroup_handle,       // The event group being updated. 
                           VOLT_MONITOR_DONE);

        // 4. Sleep - Maintain fixed execution frequency
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(VOLT_MONITOR_TASK_DELAY));
    }
}


*/