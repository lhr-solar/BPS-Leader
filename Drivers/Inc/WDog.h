#ifndef WDOG_H
#define WDOG_H

/** ===========================================================
  WatchDog Driver
===============================================================
- The Watchdog will trigger a reset sequence if it is not refreshed 
  within an expected time window. 


-----------------------------------------------------------
How to calculate COUNTDOWN value from desired refresh time
-----------------------------------------------------------
- Countdown value is how many ticks the Watchdog will count 
  from before it resets the system.


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


#include "stm32xx_hal.h"

/* ---------------------------- MACROS ----------------------------*/

#define WDOG_PRESCALAR IWDG_PRESCALER_8 
#define WDOG_COUNTDOWN 79               // Tick value before we refresh the IDWG (current value converts to ~20ms)
#define WDOG_WINDOW   0xFFF

#define WDOG_TIMEOUT_MS 20
#define TEST_REFRESH_MS 15
/* ----------------------------------------------------------------*/


/**
 * @brief Initialize the watchdog.
 * (1): Load IWDG with parameters (IWDG_COUNTDOWN, PRESCALAR)
 * (2): Initialize IWDG
 * (3): Wait until status flag is reset; check if Init failed
 * @param gpio_config GPIO_InitTypeDef structure that contains GPIO config information
 * @param errorHandler Pointer to user defined function for error handling
 */
void WDog_Init(GPIO_InitTypeDef gpio_config, void(*errorHandler)(void));

/**
 * @brief Refresh ("pet") the watchdog so it does not reset the system
 * - Reloads IDWG with countown value.
 */
void WDog_Refresh();

/**
 * @brief Check whether the watchdog has reset the system
 * @retval 1 if true (has reset); 0 is false (has not reset)
 */
int WDog_CheckIfReset();

/**
 * @brief Error Handler: Contains procedures for Watchdog failure
 */
void WDog_Error_Handler(void);

#endif