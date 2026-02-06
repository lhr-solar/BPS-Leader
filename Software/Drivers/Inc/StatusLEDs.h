#include "common.h"

/** @brief Number of bits in the module fault bitmap. */
#define MOD_FAULT_BITS 5

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
    WATCHDOG_ERR__LED,  /**< System watchdog timeout error */
    FAULT_LED_NUM,      /**< Count of standard fault LEDs */
    DEBUG_LED = 15      /**< Dedicated debug/test LED */
} Fault_Mapping_t;

/** @brief Sets a specific LED to on (true) or off (false). */
void LED_set(Fault_Mapping_t LED, bool state);

/** * @brief Updates fault LEDs based on a bitmask (1 bit per fault).
 * @param bitmap 5-bit value where each bit toggles a corresponding LED.
 */
void LEDsModFaultBitmap_set(uint8_t bitmap);

/** @brief Turns off all LEDs. */
void LEDs_clear(void);

/** @brief Configures GPIO pins for all diagnostic LEDs. */
void LEDs_init(void);