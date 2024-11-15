/** pinConfig.h
 * Config file to store common pin configurations
 */
#ifndef PINCONFIG_H__
#define PINCONFIG_H__

//--------------------------------------------------------------------------------
// Heartbeat LED
#ifdef NUCLEOF446
#define HEARTBEATPORT GPIOA
#define HEARTBEATPIN GPIO_PIN_5

#else
#define HEARTBEATPORT GPIOA
#define HEARTBEATPIN GPIO_PIN_5

#endif


 #endif