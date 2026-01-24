#include "common.h"


#define tmpHmdAdresss 0x44 // address shifted 1 bit


static void MX_I2C4_Init(void);

void readTmpHmd();
static void sensorPoll();

