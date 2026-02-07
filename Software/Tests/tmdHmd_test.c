// TEST DESCIRPTION:
/* This test should blink all fault LED's 3 times, cycle through count up from 0 to 31 in binary on the Mod Fault LEDs,
then blink DEBUG 3 times. After, it should do a wave down down the board. Ideally. */

#include "I2C_Driver.h"
#include "common.h"

static uint8_t pollCMD_test = 0xFD;

int main() {

    HAL_Init();

    SystemClock_Config();

    i2c_Init();

    


}
