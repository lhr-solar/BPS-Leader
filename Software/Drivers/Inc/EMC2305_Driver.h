#pragma once

#include "common.h"

// initialize hardware
void EMC2305_init(void);

// initialize fans and chip 
// ONLY CALL FROM TASK
void EMC2305_Driver_init();