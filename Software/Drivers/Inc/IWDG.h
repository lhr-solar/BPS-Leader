#pragma once

#include "stm32xx_hal.h"

/** ===========================================================
  Independent Watchdog (IWDG)
===============================================================
- The Watchdog will trigger a reset sequence if it is not refreshed 
  within an expected time window. 
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

// IWDG timeout must comfortably exceed the slowest monitored-task cadence, otherwise the watchdog
// resets even when everything is healthy (the pet only happens once ALL monitored tasks check in).
// The volt/temp monitors run on ~290 ms periods and can stretch to several hundred ms under
// CAN-recv timeouts, so we size the timeout to ~2 s. With prescaler 256 each reload tick is
// 256/32000 s = 8 ms, so RL=249 -> (249+1)*8 ms = 2000 ms. A genuine RTOS hang still resets the
// MCU within ~2 s. ponytail: shrink this only after the monitor tasks are given tighter cadences.
#define IWDG_PRESCALAR IWDG_PRESCALER_256
#define IWDG_COUNTDOWN_TICKS 249              // (249+1) * 8ms = ~2000ms

#define IWDG_TIMEOUT_MS 2000

// RTOS software window-timer period (ticks): the minimum spacing between pet attempts, exposed as
// one of the ALL_TASKS_DONE bits. Hardware IWDG windowing is disabled (see IWDG.c) so only the
// upper timeout above matters -- this just keeps the pet loop from spinning.
#define IWDG_WINDOW_TICKS 27
/* ----------------------------------------------------------------*/


/**
 * @brief Initializes IWDG parameters; does not start IWDG.
 * (1): Loads IWDG_COUNTDOWN and PRESCALAR
 */
void IWDG_Init();

/**
 * @brief Starts the Watchdog
 * (1): Initializes IWDG
 * (2): Waits until status flag is reset; check if Init failed
 * @param errorHandler Pointer to user defined function for error handling
 */
void IWDG_Start();

/**
 * @brief Refresh ("pet") the watchdog so it does not reset the system
 * - Reloads IDWG with countown value.
 * @retval Returns HAL status
 */
HAL_StatusTypeDef IWDG_Refresh();

/**
 * @brief Check whether the watchdog has reset the system
 * @retval 1 if true (has reset); 0 is false (has not reset)
 */
uint8_t IWDG_CheckIfReset();

/**
 * @brief Error Handler: Contains procedures for Watchdog failure
 */
void IWDG_Error_Handler(void);
