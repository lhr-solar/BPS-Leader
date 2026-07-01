#pragma once
#include "CANbus.h"

/** * @brief Disables all MPPTs by sending the set mode command to each MPPT with a mode of 0 (disabled).
 * @param delay_ms          Wait time for CAN message to be sent before returning
 * @return can_status_t     Returns CAN_OK if the command was successful, or CAN_ERR if there was an error.
 */
can_status_t disableAllMPPTs(TickType_t delay_ms);

/** * @brief Enables all MPPTs by sending the set mode command to each MPPT with a mode of 1 (enabled).
 * @param delay_ms          Wait time for CAN message to be sent before returning
 * @return can_status_t     Returns CAN_OK if the command was successful, or CAN_ERR if there was an error.
 */
can_status_t enableAllMPPTs(TickType_t delay_ms);

/** * @brief Puts all MPPTs into constant-voltage mode (set mode 2). In CV mode the MPPT holds the
 *          output (bus) at its configured voltage limit and tapers current as the bus approaches it.
 *          Set the limit first with setAllMPPTsOutputVoltageLimit().
 * @param delay_ms          Wait time for CAN message to be sent before returning
 * @return can_status_t     CAN_OK if all commands were sent, else CAN_ERR.
 */
can_status_t enableAllMPPTsConstantVoltage(TickType_t delay_ms);

/** * @brief Sets the output (bus) voltage limit on all MPPTs (SetOutputVoltageLimit, int16).
 * @param limit_raw         Raw int16 wire value -- units are the MPPT's (see MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB).
 * @param delay_ms          Wait time for CAN message to be sent before returning
 * @return can_status_t     CAN_OK if all commands were sent, else CAN_ERR.
 */
can_status_t setAllMPPTsOutputVoltageLimit(int16_t limit_raw, TickType_t delay_ms);
