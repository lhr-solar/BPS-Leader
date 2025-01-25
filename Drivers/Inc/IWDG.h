#ifndef IWDG_H
#define IWDG_H

/** ===========================================================
Independent Watch Dog (IWDG) Driver
===============================================================
- The IWDG will trigger a reset sequence if it is not refreshed 
  within an expected time window. 


-----------------------------------------------------------
How to calculate COUNTDOWN value from desired refresh time
-----------------------------------------------------------
  - Countdown / timeout value is how long the IDWG will 
    count down before resetting the system. We think of this
    in terms of seconds / milliseconds, but the IWDG will 
    hold this value in terms of a tick countdown.


-----------------------------------------------------------
Formula [finding timeout (in s) from RL]
-----------------------------------------------------------
            t_IWDG = t_LSI * 4 * 2^PR * (RL + 1)
  Parameters:
  - t_IWDG  : IDWG timeout (in seconds)
  - t_LSI   : constant; 1/32,000 (represents 31.25 uS [micro second])
  - PR      : prescalar (PRESCALAR_4 = 0, PRESCALAR_8 = 1, etc)
  - RL      : reload time (in terms of IWDG ticks)


-----------------------------------------------------------
Revised formula [finding RL from timeout (in ms)]
-----------------------------------------------------------
		RL = [(t_IWDG * 32,000) / (4 * 2^PR * 1000)] - 1
  Parameters:
  - RL      : countdown value to put for IWDG_COUNTDOWN
  - t_IWDG  : the countdown timeout you want in ms

-----------------------------------------------------------
Common Timeouts
  - 20 ms:  prescalar 8 (1), countdown 79
  - 10 ms:  prescalar 4 (0), countdown 79
  - 5 ms:   prescalar 4 (0), countdown 39
----------------------------------------------------------- */


#include "stm32xx_hal.h"

/* ------------------------------ MACROS ------------------------------*/
/** @brief Countdown value (corresponds to ms value before we 
 *  refresh the IDWG to prevent it from resetting the system. */
#define IWDG_PRESCALAR IWDG_PRESCALER_4
#define IWDG_COUNTDOWN 79 
// 20ms timeout ^

#define IWDG_TIMEOUT_MS 10
#define SYS_REFRESH_MS  8
/* ----------------------------------------------------------------*/


/**
 * @brief Initialize the watchdog.
 * (1): Load IWDG with parameters (IWDG_COUNTDOWN, PRESCALAR)
 * (2): Initialize IWDG
 * (3): Wait until status flag is reset; check if Init failed
 * @param gpio_config GPIO_InitTypeDef structure that contains GPIO config information
 * @param _ptr_errorHandler Pointer to user defined function for error handling
 */
void IWDG_Init(GPIO_InitTypeDef gpio_config, void(*_ptr_errorHandler)(void));

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
 * @brief Error Handler: Contains procedures for IWDG failure
 */
void IWDG_Error_Handler(void);

#endif