#include "common.h"
#include "CANbus.h"
#include "BPS_Tasks.h"


#define CAN_RX_FORWARD_QUEUE_SIZE    10   
#define CAN_FORWARD_WAIT_TICKS       pdMS_TO_TICKS(10)

static StaticQueue_t canRxForwardQueueBuffer;
static uint8_t canRxForwardQueueStorage[CAN_RX_FORWARD_QUEUE_SIZE * sizeof(can_rx_payload_t)];
static QueueHandle_t canRxForwardQueue;

StaticTask_t Task_Can_Forward_Buffer;
StackType_t Task_Can_Forward_Stack[TASK_CAN_FORWARD_STACK_SIZE];

// UNCOMMENT FOR CAN TEST (I don't like this either)
#define CAN_TEST

#ifdef CAN_TEST
    static volatile bool enable_bench_forwarding = true;
#endif

void can_fd_rx_callback_hook(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs, can_rx_payload_t recv_payload) 
{ 

    BaseType_t higherPriorityTaskWoken = pdFALSE;

#ifdef CAN_TEST
    if (!enable_bench_forwarding) return;       
    enable_bench_forwarding = false;
#endif

    if (hfdcan->Instance == BPS_CAN_CHANNEL) {
        if(canRxForwardQueue != NULL){
            xQueueSendCircularBufferFromISR (
                canRxForwardQueue,
                &recv_payload,
                &higherPriorityTaskWoken,
                recv_payload.header.DataLength
            );
        }
    }
}

void CanRxForwardTask_Init(void){
    canRxForwardQueue = xQueueCreateStatic(
        CAN_RX_FORWARD_QUEUE_SIZE,
        sizeof(can_rx_payload_t),
        canRxForwardQueueStorage,
        &canRxForwardQueueBuffer
    );

    if(canRxForwardQueue == NULL){
        return;
    }
}

void Task_CanRxForward(){

    // canbus MUST be initialized by now
    CanRxForwardTask_Init();

    can_rx_payload_t payload;

    uint32_t ID, data_length;
    uint8_t* data;

    while(1){
        
        if (xQueueReceive(canRxForwardQueue, &payload, portMAX_DELAY) == pdTRUE){
            ID = payload.header.Identifier;
            data_length = payload.header.DataLength;
            data = payload.data;
            
            if (car_can_send(ID, data, data_length, CAN_FORWARD_WAIT_TICKS) != CAN_OK) {
                // Panic
            }
        }
    }
}