#include "BPS_Tasks.h"
#include "StatusLEDs.h"
#include "CANbus.h"
#include "Contactors.h"
#include "DebugPrintf.h"
#include "SHT45.h"
#include "EMC2305_Driver.h"
#include "BPSCAN_can_msgs.h"

#define BLINKY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINKY_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

#define DELAY_1S pdMS_TO_TICKS(1000)

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_Voltage_Tap_Fault;
} bps_voltage_arr_t;

// Static task buffers
static StaticTask_t xBlinkyTaskBuffer;
static StackType_t xBlinkyStack[BLINKY_TASK_STACK_SIZE];

StaticTask_t xTestTaskBuffer;
StackType_t xTestStack[TEST_TASK_STACK_SIZE];

// blink
void vBlinkyTask(void *pvParameters)
{
    while (true)
    {
        toggleHeartbeat();
        vTaskDelay(DELAY_1S);
    }
}
static const uint8_t volt_id_mask = 0x1F;
/**
 * @brief Packs a bps_voltage_arr_t struct into a raw CAN data buffer.
 * * @param raw_volt_can_data Pointer to the 8-byte (or DLC size) buffer to fill.
 * @param volt_can_data Pointer to the source struct containing the tap data.
 */
static void volt_can_pack(uint8_t *raw_volt_can_data, const bps_voltage_arr_t *volt_can_data)
{

    if (raw_volt_can_data == NULL || volt_can_data == NULL)
    {
        return;
    }

    // Clear the buffer first to ensure no old data remains
    // Assuming the DLC is 4 based on your unpack logic
    for (int i = 0; i < 4; i++)
        raw_volt_can_data[i] = 0;

    // Byte 0: Tap Index (Applying the mask to ensure it doesn't bleed into other bits)
    raw_volt_can_data[0] = volt_can_data->BPS_Tap_idx & volt_id_mask;

    // Byte 1: Voltage Low Byte (Least Significant Byte)
    raw_volt_can_data[1] = (uint8_t)(volt_can_data->BPS_Voltage_Tap_Data & 0xFF);

    // Byte 2: Voltage High Byte (Most Significant Byte)
    raw_volt_can_data[2] = (uint8_t)((volt_can_data->BPS_Voltage_Tap_Data >> 8) & 0xFF);

    // Byte 3: Fault Bit/Code
    raw_volt_can_data[3] = volt_can_data->BPS_Voltage_Tap_Fault;
}

void vFakeVoltSend(void *pvParameters)
{

    vTaskDelay(pdMS_TO_TICKS(250));

    bps_voltage_arr_t message_struct = {
        .BPS_Tap_idx = 0,
        .BPS_Voltage_Tap_Data = CELL_UNDERVOLTAGE_THRESHOLD_MV,
        .BPS_Voltage_Tap_Fault = 0};

    uint8_t can_id = CAN_ID_BPS_VT0_VOLTAGE_ARR;

    uint8_t can_message[CAN_DLC_BPS_VT0_VOLTAGE_ARR];

    while (true)
    {

        vTaskDelay(pdMS_TO_TICKS(VOLT_MONITOR_TASK_DELAY_MS) / 32);

        volt_can_pack(can_message, &message_struct);

        car_can_send(can_id, can_message, CAN_DLC_BPS_VT0_VOLTAGE_ARR, 0);

        (message_struct.BPS_Tap_idx == 31) ? (message_struct.BPS_Tap_idx = 0) : message_struct.BPS_Tap_idx++;

        can_id = message_struct.BPS_Tap_idx / 4 + CAN_ID_BPS_VT0_VOLTAGE_ARR;

        message_struct.BPS_Voltage_Tap_Data = (message_struct.BPS_Voltage_Tap_Data >= CELL_OVERVOLTAGE_THRESHOLD_MV - 10) ? CELL_UNDERVOLTAGE_THRESHOLD_MV : message_struct.BPS_Voltage_Tap_Data + 10;
    }
}

int main()
{
    // Initialize the contactor hardware and software abstractions

    HAL_Init();

    SystemClock_Config();

    CAN_Init();
    
    LEDs_init();

    contactor_init();

    SHT45_init();

    EMC2305_Driver_init();

    debugPrintf_init();

    Init_WDogTask();

    printf("Initialized\n\r");

    xStateBits = xEventGroupCreateStatic(&xStateBits_buffer);

    xTaskCreateStatic(
        vFakeVoltSend,
        "Fake Voltage Measurements",
        TEST_TASK_STACK_SIZE,
        NULL,
        TEST_TASK_PRIORITY,
        xTestStack,
        &xTestTaskBuffer);

    xTaskCreateStatic(
        Task_FaultHandler,             // Task function
        "FaultHandler",                // Name of the task (for debugging)
        FAULT_HANDLER_TASK_STACK_SIZE,   // Stack size in words
        (void*)NULL,                       // Task input parameter
        TASK_FAULT_HANDLER_PRIO,       // Task priority
        FaultHandler_Task_Stack,       // Task handle
        &FaultHandler_Task_Buffer      // Static task buffer (optional)
    );

    xTaskCreateStatic(
        Task_Voltage_Monitor,
        "Voltage Task",
        TASK_VOLTAGE_MONITOR_STACK_SIZE,
        NULL,
        TASK_VOLTAGE_MONITOR_PRIO,
        Task_Voltage_Stack_Array,
        &Task_Voltage_Buffer);

    xTaskCreateStatic(
        vBlinkyTask,
        "Blinky",
        BLINKY_TASK_STACK_SIZE,
        NULL,
        BLINKY_TASK_PRIORITY,
        xBlinkyStack,
        &xBlinkyTaskBuffer);

    vTaskStartScheduler();

    return 0;
}
