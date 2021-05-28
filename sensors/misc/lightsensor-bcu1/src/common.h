//
// Created by Mario Theodoridis on 22.5.21.
//

#ifndef INC_LIGHTSENSOR_COMMON_H
#define INC_LIGHTSENSOR_COMMON_H

#include <stdint.h>
#include <sblib/eib.h>
#include <sblib/i2c/BH1750.h>

#ifdef LOGGING
    #include "log/logger.h"
    #define pretty(val) (int)(val*1)
    #define LOG(...) serPrintf(__VA_ARGS__)
#else
    #define LOG(...) // nada!
#endif

// number of seconds before we reset
#ifndef uint
typedef unsigned int uint;
#endif

#include "params.h"
#include "Threshold.h"
#include "SensorConfig.h"

void fixRamLoc(COM comNo);
void initSensor();

extern SensorConfig config;
extern SensorConf sc;

#endif //INC_LIGHTSENSOR_COMMON_H
