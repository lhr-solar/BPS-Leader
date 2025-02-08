#include "BPS_Tasks.h"
#include "stm32f4xx_hal.h"
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

// CAN transmission header
CAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];  // Buffer for transmit message
uint32_t TxMailbox;

// Configure CAN Filters
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
        printf("Error: CAN filter configuration failed\n");
    }
}

// Start CAN in Loopback Mode
void CAN_Start_Loopback(void) {
    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        printf("Error: CAN start failed\n");
        return;
    }

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        printf("Error: CAN notification activation failed\n");
        return;
    }

    // Enable loopback mode
    hcan1.Instance->MCR |= CAN_MCR_LBKM;
}

// CAN Interrupt Callback (Triggered on Message Reception)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
        printf("Received CAN Message - ID: 0x%X, DLC: %d, Data: ", RxHeader.StdId, RxHeader.DLC);
        for (int i = 0; i < RxHeader.DLC; i++) {
            printf("0x%02X ", RxData[i]);
        }
        printf("\n");

        // Store received message in buffer
        can_rx_ids[can_rx_index] = RxHeader.StdId;
        for (int i = 0; i < 8; i++) {
            can_rx_buffer[can_rx_index][i] = RxData[i];
        }

        can_rx_index = (can_rx_index + 1) % CAN_RX_BUFFER_SIZE;
    }
}

// Send a CAN message
void CAN_Send_Message(uint32_t id, uint8_t *data, uint8_t length) {
    TxHeader.StdId = id;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = length;
    TxHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, data, &TxMailbox) != HAL_OK) {
        printf("Error: CAN message transmission failed\n");
    }
}

// Task: CAN Loopback Test
void Task_CAN_Loopback_Test() {
    CAN_Filter_Config();
    CAN_Start_Loopback();

    uint32_t message_count = 0;

    while(1) {
        // Prepare test message
        uint8_t test_data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        test_data[0] = message_count & 0xFF;  // Use message count as first byte

        // Send test message
        CAN_Send_Message(0x123, test_data, 8);
        printf("Sent CAN Message - ID: 0x123, Data: ");
        for (int i = 0; i < 8; i++) {
            printf("0x%02X ", test_data[i]);
        }
        printf("\n");

        message_count++;

        // Wait for reception (handled by interrupt)
        vTaskDelay(pdMS_TO_TICKS(1000));  // Send a message every second
    }
}
