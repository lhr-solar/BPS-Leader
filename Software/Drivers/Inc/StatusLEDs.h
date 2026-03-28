#pragma once

#include "common.h"

/** * @brief Number of bits in the module fault bitmap. 
 */
#define MOD_FAULT_BITS 5

/** * @brief LED States  
 */
typedef enum {
    LED_OFF = 0,
    LED_ON = 1,
    LED_OK,         // return value for function
    LED_ERR         // return value for function
} led_state_t;


/** * @brief Logic-to-Hardware mapping for diagnostic LEDs.
 * @note Values correspond to specific shift register positions.
 */
typedef enum {
    HEARTBEAT_LED,      /**< System status heartbeat */
    FAULT_LED,          /**< General system fault */
    OVER_V_LED,         /**< Over-voltage indicator */
    LOW_V_LED,          /**< Low-voltage indicator */
    OVER_AMP_LED,       /**< Over-current indicator */
    OVER_TEMP_LED,      /**< Over-temperature indicator */
    CHARGING_LED,       /**< Battery charging status */
    VTEMP_IN_LED,       /**< Voltage/Temp input active */
    AMP_IN_LED,         /**< Amperage input active */
    WATCHDOG_ERR_LED,   /**< System watchdog timeout error */
    FAULT_LED_NUM,      /**< Count of standard fault LEDs */
    DEBUG_LED = 15      /**< Dedicated debug/test LED */
} led_mapping_t;

/** * @brief Sets a specific LED to on (true) or off (false). 
 * * @param LED   The specific LED to control (mapped via led_mapping_t).
 * @param state The desired state for the LED (true for ON, false for OFF).
 * @return      led_state_t Returns LED_OK on successful update, LED_ERR on failure.
 */
led_state_t LED_set(led_mapping_t LED, bool state);

/** * @brief Updates fault LEDs based on a bitmask (1 bit per fault).
 * * @param bitmap 5-bit value where each bit toggles a corresponding fault LED.
 * @return       led_state_t Returns LED_OK on successful update, LED_ERR on failure.
 */
led_state_t LEDsModFaultBitmap_set(uint8_t bitmap);

/** * @brief Turns off all LEDs. 
 */
void LEDs_clear(void);

/** * @brief Configures GPIO pins and hardware registers for all diagnostic LEDs. 
 */
void LEDs_init(void);

/** * @brief Sets the LSOM heartbeat LED to a specific state.
 * * @param state The desired state for the heartbeat LED (true for ON, false for OFF).
 */
void setHeartbeat(bool state);

/** * @brief Toggles the current state of the LSOM heartbeat LED.
 * * Reads the current state of the heartbeat LED and flips it (ON to OFF, or OFF to ON).
 */
void toggleHeartbeat(void);