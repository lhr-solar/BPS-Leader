#ifndef IWDG_H
#define IWDG_H
// #include "stm32f4xx_hal.h"

/** @brief How long we want to wait (in ms) before we refresh 
 *  the IDWG to prevent it from resetting the system.
 *  Should be lower than the refresh time of the system itself.
 */
// extern int IWDG_REFRESH_TIME;

/**
 * @brief Initialize the watchdog
 */
void IWDG_Init();

/**
 * @brief Refresh ("pet") the watchdog so it does not reset the system
 */
void IWDG_Refresh();

/**
 * @brief Reset the watchdog
 */
void IWDG_Reset();

/**
 * @brief Check whether the watchdog has been tripped
 */
void IWDG_CheckStatus();

void Error_Handler(void);

#endif