#ifndef WDOG_H
#define WDOG_H

#include "stm32xx_hal.h"

/** ===========================================================
  WatchDog Driver
===============================================================
- The Watchdog will trigger a reset sequence if it is not refreshed 
  within an expected time window. 


-----------------------------------------------------------
  INDEPENDENT WATCHDOG
-----------------------------------------------------------
- Countdown value is how many ticks the Watchdog will count 
  from before it resets the system.


Calculating COUNTDOWN for IWDG
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
- 20 ms:  countdown 79, prescalar 8 (1)
- 10 ms:  countdown 79, prescalar 4 (0)
- 5 ms:   countdown 39, prescalar 4 (0)
----------------------------------------------------------- */


/* ---------------------------- MACROS ----------------------------*/

#define WDOG_PRESCALAR IWDG_PRESCALER_8 
#define WDOG_COUNTDOWN 79               // Tick value before we refresh the IDWG (current value converts to ~20ms)
// #define WDOG_WINDOW   0xFFF             // If using windowed watchdog (WWDG)

#define WDOG_TIMEOUT_MS 20
#define WDOG_WINDOW_MS  8
/* ----------------------------------------------------------------*/


/**
 * @brief Initialize the watchdog.
 * (1): Load IWDG with parameters (IWDG_COUNTDOWN, PRESCALAR)
 * (2): Initialize IWDG
 * (3): Wait until status flag is reset; check if Init failed
 * @param errorHandler Pointer to user defined function for error handling
 */
void WDog_Init(void(*errorHandler)(void));

/**
 * @brief Refresh ("pet") the watchdog so it does not reset the system
 * - Reloads IDWG with countown value.
 * @retval Returns HAL status
 */
HAL_StatusTypeDef WDog_Refresh();

/**
 * @brief Check whether the watchdog has reset the system
 * @retval 1 if true (has reset); 0 is false (has not reset)
 */
uint8_t WDog_CheckIfReset();

/**
 * @brief Error Handler: Contains procedures for Watchdog failure
 */
void WDog_Error_Handler(void);

#endif