#ifndef IWDG_H
#define IWDG_H
// #include "stm32f4xx_hal.h"

/** @brief How long we want to wait (in ms) before we refresh 
 *  the IDWG to prevent it from resetting the system.
 *  Should be lower than the refresh time of the system itself.
 */
// extern int IWDG_REFRESH_TIME;

/**
 * @brief Initialize the watchdog.
 * (1): Load IWDG with parameters (IWDG_COUNTDOWN, PRESCALAR)
 * (2): Initialize IWDG
 * (3): Wait until status flag is reset; check if Init failed
 */
void IWDG_Init();

/**
 * @brief Refresh ("pet") the watchdog so it does not reset the system
 * - Reloads IDWG with countown value.
 */
void IWDG_Refresh();

/**
 * @brief Check whether the watchdog has reset the system
 * @retval 1 if true (has reset); 0 is false (has not reset)
 */
int IWDG_CheckIfReset();

/**
 * @brief Error Handler: Contains procedures for when IWDG_Init fails
 */
void Error_Handler(void);

#endif