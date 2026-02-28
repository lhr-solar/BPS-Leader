#include "common.h"

/** @brief I2C address for the SHT4x sensor (0x44 << 1). */
#define tmpHmdAdresss 0x88 

/** @brief Command size sent to the sensor (typically 1 byte). */
#define tx_size 1
/** @brief Size of data returned (2 bytes temp + 1 CRC + 2 bytes hmd + 1 CRC). */
#define rx_size 6

/** @brief Global I2C communication timeout in milliseconds. */
#define I2C_TIMEOUT 100u

typedef enum {
    SHT45_OK,
    SHT45_ERR
} SHT45_status_t;

/** @brief Container for processed environmental data. */
typedef struct {
    uint32_t temp;     /**< Temperature in degrees Celsius. */
    uint32_t humidity; /**< Relative humidity percentage (0-100%). */
} tempHmdData;

/** @brief Hardware initialization for the I2C4 peripheral. */
void MX_I2C4_Init(void);

/** * @brief Triggers a sensor measurement and populates the raw buffer. 
 * @param tmpHmdBuffer Pointer to an array of size rx_size to store raw I2C data.
 */
SHT45_status_t tmpHmd_get(uint32_t *tmpHmdBuffer);

/** * @brief Custom callback for SHT4x non-blocking I2C completion. 
 * Handles internal state transitions after data is successfully received.
 */
void  SHT4x_I2C_MasterTxRxCpltCallback(void);

// intialize semaphore
void SHT4x_init();


