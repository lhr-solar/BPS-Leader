#pragma once

#include "common.h"
#include "EMC2305.h"

// initialize hardware
void EMC2305_I2C_init(void);

// initialize fans and chip 
// ONLY CALL FROM TASK
void EMC2305_Driver_init();