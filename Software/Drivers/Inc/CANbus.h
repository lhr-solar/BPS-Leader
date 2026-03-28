#pragma once

#include "common.h"
#include "CAN_FD.h"

/** * @brief Transmits a message over the CAR CAN bus.
 * @note Automatically converts the millisecond delay into RTOS ticks.
 * * @param tx_header Pointer to the configured FDCAN_TxHeaderTypeDef structure.
 * @param data      Pointer to the payload data array to be transmitted.
 * @param delay_ms  Maximum time in milliseconds to block waiting for space in the TX queue.
 */
#define car_can_send(tx_header, data, delay_ms) can_fd_send(car_can, tx_header, data, pdMS_TO_TICKS(delay_ms))

/** * @brief Transmits a message over the BPS CAN bus.
 * @note Automatically converts the millisecond delay into RTOS ticks.
 * * @param tx_header Pointer to the configured FDCAN_TxHeaderTypeDef structure.
 * @param data      Pointer to the payload data array to be transmitted.
 * @param delay_ms  Maximum time in milliseconds to block waiting for space in the TX queue.
 */
#define bps_can_send(tx_header, data, delay_ms) can_fd_send(bps_can, tx_header, data, pdMS_TO_TICKS(delay_ms))

/** * @brief Receives a specific message from the CAR CAN bus.
 * @note Automatically converts the millisecond delay into RTOS ticks.
 * * @param id        The specific CAN message ID to wait for and retrieve.
 * @param rx_header Pointer to an FDCAN_RxHeaderTypeDef to store the incoming message metadata.
 * @param data      Pointer to a buffer to store the incoming payload data.
 * @param delay_ms  Maximum time in milliseconds to block waiting for the message to arrive.
 */
#define car_can_recv(id, rx_header, data, delay_ms) can_fd_recv(car_can, id, rx_header, data, pdMS_TO_TICKS(delay_ms))

/** * @brief Receives a specific message from the BPS CAN bus.
 * @note Automatically converts the millisecond delay into RTOS ticks.
 * * @param id        The specific CAN message ID to wait for and retrieve.
 * @param rx_header Pointer to an FDCAN_RxHeaderTypeDef to store the incoming message metadata.
 * @param data      Pointer to a buffer to store the incoming payload data.
 * @param delay_ms  Maximum time in milliseconds to block waiting for the message to arrive.
 */
#define bps_can_recv(id, rx_header, data, delay_ms) can_fd_recv(bps_can, id, rx_header, data, pdMS_TO_TICKS(delay_ms))

/** * @brief External handle for the FDCAN1 peripheral (routed to BPS).
 */
extern FDCAN_HandleTypeDef* hfdcan1;

/** * @brief External handle for the FDCAN3 peripheral (routed to CAR).
 */
extern FDCAN_HandleTypeDef* hfdcan3;

/** * @brief Initializes and starts both the BPS and CAR CAN nodes simultaneously. 
 */
can_status_t CAN_Init(void);

/** * @brief Initializes a standard Transmit (TX) Header for FDCAN communication.
 * @note  USE MACROS FOR DATA LENGTH! (e.g., FDCAN_DLC_BYTES_8). Do not pass 
 * raw integer values for the length.
 * * @param tx_header  Pointer to the FDCAN_TxHeaderTypeDef structure to configure.
 * @param ID         The CAN message identifier (Standard or Extended).
 * @param dataLength The Data Length Code (DLC) macro representing the payload size.
 */
void FDCAN_Init_TXHeader(FDCAN_TxHeaderTypeDef* tx_header, uint32_t ID, uint32_t dataLength);