#include "common.h"
#include "CANbus.h"
#include "BPS_Tasks.h"

#define CAN_RX_FORWARD_QUEUE_SIZE 50
#define CAN_FORWARD_WAIT_TICKS pdMS_TO_TICKS(10)

#define FAULT_COUNT_ERROR_THRESHOLD 5

// Forward the ENTIRE BPS bus to Car CAN. Default OFF: only the pack current is forwarded (it has
// no other path to Car CAN). Enabling mirrors every VT voltage/temp frame too -- heavy Car-CAN
// traffic that the monitor tasks already cover via their aggregate messages.
#ifndef FULL_CAN_FORWARDING
#define FULL_CAN_FORWARDING 0
#endif

static StaticQueue_t canRxForwardQueueBuffer;
static uint8_t canRxForwardQueueStorage[CAN_RX_FORWARD_QUEUE_SIZE * sizeof(can_rx_payload_t)];
static QueueHandle_t canRxForwardQueue;
 
// Overrides the driver's weak can_fd_rx_callback_hook (CAN_FD.c). The name MUST match exactly
// or the override silently won't link and forwarding stops. Runs in the RX ISR for every frame.
void can_fd_rx_callback_hook(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs, can_rx_payload_t recv_payload)
{

    BaseType_t higherPriorityTaskWoken = pdFALSE;

    if ((hfdcan->Instance == BPS_CAN_CHANNEL) && (canRxForwardQueue != NULL))
    {
#if FULL_CAN_FORWARDING
        const bool forward = true; // mirror the whole BPS bus to Car CAN
#else
        const bool forward = (recv_payload.header.Identifier == CAN_ID_BPS_PACK_CURRENT);
#endif
        if (forward)
        {
            xQueueSendCircularBufferFromISR(
                canRxForwardQueue,
                &recv_payload,
                &higherPriorityTaskWoken,
                sizeof(can_rx_payload_t));
        }
    }

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// NOTE: any CAN forwards that require more processing than simply forwarding (i.e. packing the voltage aggregate msg) will be done in their respective tasks
void Task_CanRxForward()
{

    // canbus MUST be initialized by now
    can_rx_payload_t payload;

    uint32_t ID, data_length;
    uint8_t *data;

    canRxForwardQueue = xQueueCreateStatic(
        CAN_RX_FORWARD_QUEUE_SIZE,
        sizeof(can_rx_payload_t),
        canRxForwardQueueStorage,
        &canRxForwardQueueBuffer);

    if (canRxForwardQueue == NULL)
    {
        set_faultBit(BPS_CAN_ERROR);
        vTaskDelete(NULL);
    }

    while (1)
    {
        if (xQueueReceive(canRxForwardQueue, &payload, portMAX_DELAY) == pdTRUE)
        {
            ID = payload.header.Identifier;            
            data_length = payload.header.DataLength;
            data = payload.data;

            car_can_send(ID, data, data_length, CAN_FORWARD_WAIT_TICKS);
            taskYIELD();
        } 
    }
}
