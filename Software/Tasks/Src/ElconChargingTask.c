#include "BPS_Tasks.h"
#include "config.h"
#include "CANbus.h"
#include "CarCAN_can_msgs.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"
#include "BPS_Tasks.h"

#define ELCON_TASK_PERIOD_MS       100u
#define ELCON_CHARGER_TIMEOUT_MS   500u

// Charger Interface Status bit positions (little-endian, 5-byte message)
// Bit 32 = byte[4] bit 0: Comm_ok  (1 = yes, 0 = no)
// Bit 33 = byte[4] bit 1: Fault present (1 = yes, 0 = no)
#define CHARGER_COMM_OK_MASK       (1u << 0)
#define CHARGER_FAULT_PRESENT_MASK (1u << 1)

StackType_t  Task_Elcon_Charging_Stack[TASK_ELCON_CHARGING_STACK_SIZE];
StaticTask_t Task_Elcon_Charging_Buffer;
TaskHandle_t Elcon_Charging_Task;

static volatile bool  charger_timeout_fault = false;
static TimerHandle_t  xChargerTimeoutTimer;
static StaticTimer_t  xChargerTimeoutBuffer;

/**
 * @brief One-shot timer callback fires when no charger message is received
 *        within ELCON_CHARGER_TIMEOUT_MS. Sets the timeout fault and FAULT_LED.
 */
static void charger_timeout_callback(TimerHandle_t xTimer)
{
    (void)xTimer;

    charger_timeout_fault = true;
}
    

void Task_Elcon_Charging()
{
    Elcon_Charging_Task = xTaskGetCurrentTaskHandle();

    // One-shot 500 ms timeout timer — reset every time a charger message arrives
    xChargerTimeoutTimer = xTimerCreateStatic(
        "ChargerTimeout",
        pdMS_TO_TICKS(ELCON_CHARGER_TIMEOUT_MS),
        pdFALSE,                      // one-shot, not auto-reload
        NULL,
        charger_timeout_callback,
        &xChargerTimeoutBuffer);
    xTimerStart(xChargerTimeoutTimer, 0);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ELCON_TASK_PERIOD_MS));
        
        
        // Heartbeat: toggle every cycle to show the task is alive
        toggleHeartbeat();

        // Try to receive Charger Interface Status from car CAN
        uint8_t charger_buf[CAN_DLC_CHARGER_INTERFACE_STATUS] = {0};
        bool got_charger_msg = (car_can_recv(CAN_ID_CHARGER_INTERFACE_STATUS,
                                             charger_buf,
                                             CAN_DLC_CHARGER_INTERFACE_STATUS,
                                             ELCON_TASK_PERIOD_MS) == CAN_OK);

        //LED_set(FAULT_LED, charger_timeout_fault);

        if (got_charger_msg)
        {
            // Reset the 500 ms watchdog timer on every valid charger message
            xTimerReset(xChargerTimeoutTimer, 0);

            // Clear timeout fault once messages resume
            if (charger_timeout_fault)
            {
                charger_timeout_fault = false;
            }

            // Toggle CHARGING_LED to show charger messages are arriving
            static bool charging_led_state = false;
            charging_led_state = !charging_led_state;
            //LED_set(CHARGING_LED, charging_led_state);
        }

        // Extract status bits from byte 4 (bits 32 and 33)
        bool comm_ok       = (charger_buf[4] & CHARGER_COMM_OK_MASK) != 0;
        bool fault_present = (charger_buf[4] & CHARGER_FAULT_PRESENT_MASK) != 0;



        // Send BPS_CHARGE_OK every cycle; NOT_OK if timeout fault or any bad condition
        uint8_t status_buf[CAN_DLC_BPS_STATUS] = {0};
        if (charger_timeout_fault || !got_charger_msg || !comm_ok || fault_present)
        {
            status_buf[1] = BPS_STATUS_BPS_CHARGE_OK_NOT_OK;

        }
        else
        {
            status_buf[1] = BPS_STATUS_BPS_CHARGE_OK_OK;

        }

        // Flash VTEMP_IN_LED to indicate each BPS send
        static bool BPS_led_state = false;
        BPS_led_state = !BPS_led_state;
        LED_set(VTEMP_IN_LED, BPS_led_state);
        car_can_send(CAN_ID_BPS_STATUS, status_buf, CAN_DLC_BPS_STATUS, ELCON_TASK_PERIOD_MS);
    }
}