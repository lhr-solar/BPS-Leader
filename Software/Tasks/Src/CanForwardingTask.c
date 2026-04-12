#include "common.h"
#include "CANbus.h"
#include "BPS_Tasks.h"
#include "StatusLEDs.h"

#define CAN_RX_FORWARD_QUEUE_SIZE 50
#define CAN_FORWARD_WAIT_TICKS pdMS_TO_TICKS(10)
#define CAN_FORWARD_TASK_DELAY_MS 4

#define FAULT_COUNT_ERROR_THRESHOLD 5

static StaticQueue_t canRxForwardQueueBuffer;
static uint8_t canRxForwardQueueStorage[CAN_RX_FORWARD_QUEUE_SIZE * sizeof(can_rx_payload_t)];
static QueueHandle_t canRxForwardQueue;


void can_fd_rx_callback_hook(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs, can_rx_payload_t recv_payload)
{

    BaseType_t higherPriorityTaskWoken = pdFALSE;


    if (hfdcan->Instance == BPS_CAN_CHANNEL)
    {
        if (canRxForwardQueue != NULL)
        {
            xQueueSendCircularBufferFromISR(
                canRxForwardQueue,
                &recv_payload,
                &higherPriorityTaskWoken,
                recv_payload.header.DataLength);
        }
        toggleHeartbeat();

    }
}

void CanRxForwardTask_Init(void)
{


    canRxForwardQueue = xQueueCreateStatic(
        CAN_RX_FORWARD_QUEUE_SIZE,
        sizeof(can_rx_payload_t),
        canRxForwardQueueStorage,
        &canRxForwardQueueBuffer);

    if (canRxForwardQueue == NULL)
    {
        return;
    }
}

// NOTE: any CAN forwards that require more processing than simply forwarding (i.e. packing the voltage aggregate msg) will be done in their respective tasks
void Task_CanRxForward()
{

    uint32_t fault_count = 0;

    // canbus MUST be initialized by now
    CanRxForwardTask_Init();

    can_rx_payload_t payload;

    uint32_t ID, data_length;
    uint8_t *data;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CAN_FORWARD_TASK_DELAY_MS));

        if (xQueueReceive(canRxForwardQueue, &payload, portMAX_DELAY) == pdTRUE)
        {

            ID = payload.header.Identifier;            
            data_length = payload.header.DataLength;
            data = payload.data;


            if (car_can_send(ID, data, data_length, CAN_FORWARD_WAIT_TICKS) != CAN_OK)
            {
                fault_count++;
            }
            else
                fault_count = 0;
        }
    }
}
