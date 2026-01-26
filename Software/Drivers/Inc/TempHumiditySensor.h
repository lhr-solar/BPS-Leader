#include "common.h"

// adress of sensor (should be shifted lelft whenever used)
#define tmpHmdAdresss 0x44 

#define tx_size 1
#define rx_size 6

#define I2C_TIMEOUT 100u


typedef struct {

    double temp;
    double humidity;

} tempHmdData;


void MX_I2C4_Init(void);

void readTmpHmd(double* tmpHmdBuffer);

void SHT4x_I2C_MasterTxRxCpltCallback();


