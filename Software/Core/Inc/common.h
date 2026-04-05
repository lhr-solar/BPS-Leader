#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
#include "stm32xx_hal.h"
#include <timers.h>
#include <stdio.h>
#include <stdbool.h>
#include "config.h"

#include "pindef.h"

#include "faultHandler.h"

typedef enum {
    NORMAL = 0,   // We good
    EMERGENCY = 1  // We bad
} fault_state_t;    

void Fault_Handler(void);
void Error_Handler(void);
void SystemClock_Config(void);

TickType_t Calculate_TimeDifference(TickType_t newTime, TickType_t oldTime);



