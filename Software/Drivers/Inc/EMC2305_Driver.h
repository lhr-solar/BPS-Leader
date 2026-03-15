#pragma once

#include "common.h"
#include "EMC2305.h"

#define FAN_MIN_RPM 3200
#define FAN_MAX_RPM 8000

// initialize hardware
void EMC2305_I2C_init(void);

// initialize fans and chip 
// ONLY CALL FROM TASK
void EMC2305_Driver_init();

// sets fans to max in case of fault
void set_fans_MAX(void);