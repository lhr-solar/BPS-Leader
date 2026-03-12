#pragma once

#include <stdint.h>

// 12-bit ADC Lookup Table
// Output Units: millivolts (mV)
// Range: 0 mV to 332823 mV

extern const uint32_t Battery_LUT[4096];