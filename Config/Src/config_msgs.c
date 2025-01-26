// For displaying the value
// Two defines needed to print the actual value and not the name of the define.
#include "config.h"

#define PRE(s)  "\r    "s"  "   // \r removes the filepath and 'note: '#pragma message:...' parts
#define STR(x)  #x
#define XSTR(x) STR(x)

#pragma message(PRE("⚡") "NUM_BATTERY_MODULES set to                " XSTR(NUM_BATTERY_MODULES))

#pragma message(PRE("🔥") "NUM_TEMPERATURE_SENSORS set to            " XSTR(NUM_TEMPERATURE_SENSORS))