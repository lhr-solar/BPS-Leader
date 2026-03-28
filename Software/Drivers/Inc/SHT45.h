#pragma once

// DATASHEET OF SHT4x: https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf

#include "common.h"

extern I2C_HandleTypeDef hi2c4;

/** * @brief I2C address for the SHT4x sensor (0x44 << 1). 
 */
#define tmpHmdAdresss (0x44 << 1)

/** * @brief Command size sent to the sensor (typically 1 byte). 
 */
#define TX_SIZE 1

/** * @brief Size of data returned (2 bytes temp + 1 CRC + 2 bytes hmd + 1 CRC). 
 */
#define RX_SIZE 6

/** * @brief Global I2C communication timeout in milliseconds. 
 */
#define SHT45_TIMEOUT_MS 100u

/**
 * @brief Status codes for SHT45 operations.
 */
typedef enum {
    SHT45_OK,   /**< Operation completed successfully */
    SHT45_ERR   /**< Operation failed (e.g., I2C error or timeout) */
} SHT45_status_t;

/**
 * @brief Measurement types available from the SHT45 sensor.
 */
typedef enum {
    SHT45_TEMP,     /**< Temperature measurement */
    SHT45_HUMIDITY  /**< Humidity measurement */
} SHT45_measurement_type_t;

/** * @brief Hardware initialization for the I2C peripheral communicating with the SHT4x. 
 */
void SHT45_init(void);

/** * @brief Triggers a sensor measurement and populates the raw buffer. 
 * * @param tmpHmdBuffer Pointer to an array (expected size RX_SIZE) to store raw I2C data.
 * * @param delay_ms delay in ms to wait for the I2C transactions to complete
 * @return SHT45_status_t Returns SHT45_OK on success, SHT45_ERR on failure.
 */
SHT45_status_t SHT45_get(int32_t *tmpHmdBuffer, TickType_t delay_ms);

/** * @brief Custom callback for SHT4x non-blocking I2C completion. 
 * * Handles internal state transitions after data is successfully received 
 * from the I2C peripheral.
 */
void SHT45_I2C_MasterTxRxCpltCallback(void);