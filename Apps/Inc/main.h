#ifndef _MAIN_H
#define _MAIN_H


/** @brief How long we want to wait (in ms) before we "kick" (aka refresh)
 *  the IDWG to prevent it from resetting the system.
 *  Should be smaller than the refresh time of the system itself.
 */
int IDWG_KICK_TIME;

void Error_Handler(void);

#endif