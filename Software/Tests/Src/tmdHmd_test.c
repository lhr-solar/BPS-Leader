#include "I2C_Driver.h"
#include "common.h"

static uint8_t pollCMD_test = 0xFD;

int main() {

    HAL_Init();

    SystemClock_Config();

    i2c_Init();

    


}
