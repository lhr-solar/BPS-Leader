#include "common.h"

// adress of sensor shifted left 1 bit
#define tmpHmdAdresss 0x88 

#define tx_size 1
#define rx_size 6

#define I2C_TIMEOUT 100u

typedef struct {

    double temp;
    double humidity;

} tempHmdData;

void MX_I2C4_Init(void);

void tmpHmd_get(uint16_t *tmpHmdBuffer);

void SHT4x_I2C_MasterTxRxCpltCallback();


