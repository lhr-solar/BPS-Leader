#ifndef IWDG_H
#define IWDG_H
#include "stm32f4xx_hal.h"

/** @brief How long we want to wait (in ms) before we refresh 
 *  the IDWG to prevent it from resetting the system.
 *  Should be lower than the refresh time of the system itself.
 */
// extern int IWDG_REFRESH_TIME;

void IWDG_Init();
void IWDG_Refresh();
void IWDG_Reset();
void IWDG_CheckStatus();

void Error_Handler(void);

#endif