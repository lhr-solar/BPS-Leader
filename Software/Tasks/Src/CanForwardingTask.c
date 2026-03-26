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

    if (hfdcan->Instance != BPS_CAN_CHANNEL) return;

#ifdef CAN_TEST
    if (!enable_bench_forwarding) return;       
    enable_bench_forwarding = false;
#endif

    if(canRxForwardQueue != NULL){
        
        xQueueSendFromISR(
            canRxForwardQueue,
            &recv_payload,
            &higherPriorityTaskWoken
        );

    }

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
    
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

    FDCAN_TxHeaderTypeDef tx_forward_header;
    uint32_t ID;

    while(1){
        
        if (xQueueReceive(canRxForwardQueue, &payload, portMAX_DELAY) == pdTRUE){
            ID = payload.header.Identifier;
            FDCAN_Init_TXHeader(&tx_forward_header, ID, payload.header.DataLength);
            
            if (can_fd_send(hfdcan3, &tx_forward_header, payload.data, CAN_FORWARD_WAIT_TICKS) != CAN_OK) {
                // Panic
            }
        }
    }
}