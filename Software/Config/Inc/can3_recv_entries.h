#include "CarCAN_can_msgs.h"
#include "BPSCAN_can_msgs.h"

CAN_RECV_ENTRY(CAN_ID_DRIVER_INPUT_STATUS, 1, true)

// BPS command + module override inputs (from Controls) - circular so only the newest state is kept
CAN_RECV_ENTRY(CAN_ID_BPS_COMMAND, CAN_DLC_BPS_COMMAND, true)
CAN_RECV_ENTRY(CAN_ID_BPS_MODULE_OVERRIDE, CAN_DLC_BPS_MODULE_OVERRIDE, true)