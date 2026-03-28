#pragma once

#include "common.h"
#include "CAN_FD.h"

/** * @brief External handle for the FDCAN1 peripheral (routed to BPS).
 */
extern FDCAN_HandleTypeDef* hfdcan1;

/** * @brief External handle for the FDCAN3 peripheral (routed to CAR).
 */
extern FDCAN_HandleTypeDef* hfdcan3;

extern FDCAN_HandleTypeDef* car_can;
extern FDCAN_HandleTypeDef* bps_can;

/** * @brief Transmits a message over the CAR CAN bus.
 * @note USE MACROS FOR CAN DATA LENGTH!! (i.e. FDCAN_DLC_BYTES_1)
 * * @param ID        CAN message ID
 * @param data        Pointer to the payload data array to be transmitted.
 * @param data_length Length of message to be sent in bytes
 * @param delay_ms    Maximum time in milliseconds to block waiting for space in the TX queue.
 */
can_status_t car_can_send(uint32_t ID, uint8_t data[], uint32_t data_length, TickType_t delay_ms);

/** * @brief Transmits a message over the BPS CAN bus.
 * @note USE MACROS FOR CAN DATA LENGTH!! (i.e. FDCAN_DLC_BYTES_1)
 * * @param ID        CAN message ID
 * @param data        Pointer to the payload data array to be transmitted.
 * @param data_length Length of message to be sent in bytes
 * @param delay_ms    Maximum time in milliseconds to block waiting for space in the TX queue.
 */
can_status_t bps_can_send(uint32_t ID, uint8_t data[], uint32_t data_length, TickType_t delay_ms);

/** * @brief Receives a specific message from the CAR CAN bus.
 * @note USE MACROS FOR CAN DATA LENGTH!! (i.e. FDCAN_DLC_BYTES_1)
 * * @param id        The specific CAN message ID to wait for and retrieve.
 * @param data      Pointer to a buffer to store the incoming payload data.
 * @param data_length Length of message to be sent in bytes
 * @param delay_ms  Maximum time in milliseconds to block waiting for the message to arrive.
 */
can_status_t car_can_recv(uint32_t ID, uint8_t data[], uint32_t data_length, TickType_t delay_ms);

/** * @brief Receives a specific message from the BPS CAN bus.
 * @note USE MACROS FOR CAN DATA LENGTH!! (i.e. FDCAN_DLC_BYTES_1)
 * * @param id        The specific CAN message ID to wait for and retrieve.
 * @param data      Pointer to a buffer to store the incoming payload data.
 * @param data_length Length of message to be sent in bytes
 * @param delay_ms  Maximum time in milliseconds to block waiting for the message to arrive.
 */
can_status_t bps_can_recv(uint32_t ID, uint8_t data[], uint32_t data_length, TickType_t delay_ms);

/** * @brief Initializes and starts both the BPS and CAR CAN nodes simultaneously. 
 */
can_status_t CAN_Init(void);
