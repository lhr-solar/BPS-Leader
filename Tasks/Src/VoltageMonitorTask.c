#include "BPS_Tasks.h"
#include "stm32xx_hal.h"
#include <stdio.h>

// External CAN handle
extern CAN_HandleTypeDef hcan1;

// Buffer to store received CAN messages
#define CAN_RX_BUFFER_SIZE 10  // Store 10 messages
uint8_t can_rx_buffer[CAN_RX_BUFFER_SIZE][8];  // Each message is max 8 bytes
uint32_t can_rx_ids[CAN_RX_BUFFER_SIZE];  // Store message IDs
uint8_t can_rx_index = 0;  // Current buffer index

// CAN reception header
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];  // Buffer for received message

// 🔹 Configure CAN Filters
void CAN_Filter_Config(void) {
    CAN_FilterTypeDef canfilter;

    canfilter.FilterBank = 0;
    canfilter.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilter.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilter.FilterIdHigh = 0x0000;   // Accept all messages
    canfilter.FilterIdLow = 0x0000;
    canfilter.FilterMaskIdHigh = 0x0000;
    canfilter.FilterMaskIdLow = 0x0000;
    canfilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    canfilter.FilterActivation = ENABLE;
    canfilter.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &canfilter) != HAL_OK) {
        // Handle error
    }
}

// 🔹 Start CAN Reception (Enable Interrupts)
void CAN_Start(void) {
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

// 🔹 CAN Interrupt Callback (Triggered on Message Reception)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
        // Print received message before storing
        printf("Received CAN Message - ID: 0x%X, DLC: %d, Data: ", RxHeader.StdId, RxHeader.DLC);
        for (int i = 0; i < RxHeader.DLC; i++) {
            printf("0x%02X ", RxData[i]);
        }
        printf("\n");

        // Store received message in buffer
        can_rx_ids[can_rx_index] = RxHeader.StdId;  // Store message ID
        for (int i = 0; i < 8; i++) {
            can_rx_buffer[can_rx_index][i] = RxData[i];
        }

        // Update buffer index (circular buffer)
        can_rx_index = (can_rx_index + 1) % CAN_RX_BUFFER_SIZE;
    }
}

// 🔹 Task: CAN Receiver (FreeRTOS Task)
void Task_CAN_Receive() {
    CAN_Filter_Config();  // Configure filters
    CAN_Start();  // Start CAN reception

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(100));  // Just idle, ISR handles reception
    }
}
