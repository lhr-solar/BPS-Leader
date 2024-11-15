/** config.h
 * Config file to hold any aliases/constants referenced by multiple files
 * Not specific to a single board/unit
 */

#ifndef CONFIG_H__
#define CONFIG_H__

typedef enum ErrorStatus_e {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

//--------------------------------------------------------------------------------
// Battery Pack layout
#ifndef NUM_BATTERY_MODULES
#define NUM_BATTERY_MODULES             31      // Number of battery modules
#endif


#endif