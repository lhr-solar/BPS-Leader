#ifndef IWDG_H
#define IWDG_H

/** @brief Countdown value (corresponds to ms value) before we 
 *  refresh the IDWG to prevent it from resetting the system. */
#define IWDG_COUNTDOWN 79 
// 10ms timeout ^


#define IWDG_TIMEOUT_MS 10
#define SYS_REFRESH_MS  8

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