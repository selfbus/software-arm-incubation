//
// Created by mario on 17.05.20.
//

#ifndef INC_4SENSE_COMMON_H
#define INC_4SENSE_COMMON_H

#include <stdint.h>
#include <sblib/eib.h>
#include <sblib/sensors/ds18x20.h>

#ifdef LOGGING
    #include "log/logger.h"
    #define pretty(val) (int)(val/1.6)
    #define LOG(...) serPrintf(__VA_ARGS__)
#else
    #define LOG(...) // nada!
#endif

#define NUM_SENSORS     4
#ifndef uint
typedef unsigned int uint;
#endif

#include "params.h"
#include "Threshold.h"
#include "SensorConfig.h"

extern SensorConfig configs[NUM_SENSORS];


#endif //INC_4SENSE_COMMON_H
