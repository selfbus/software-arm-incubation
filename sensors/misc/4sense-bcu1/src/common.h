//
// Created by Mario Theodoridis on 17.05.20.
//

#ifndef INC_4SENSE_COMMON_H
#define INC_4SENSE_COMMON_H

#include <stdint.h>
#include <sblib/eib.h>
#include <sblib/sensors/ds18x20.h>
#include <sblib/sensors/dht.h>

#ifdef LOGGING
    #include "log/logger.h"
    #define pretty(val) (int)(val*10)
    #define LOG(...) serPrintf(__VA_ARGS__)
#else
    #define LOG(...) // nada!
#endif

// number of seconds before we reset
#define RE_INIT_DELAY   2000
#define NUM_SENSORS     4
#ifndef uint
typedef unsigned int uint;
#endif

#include "params.h"
#include "Threshold.h"
#include "SensorConfig.h"

void fixRamLoc(COM comNo);
void initSensors();
#define requestSensorInit() { needSensorInit = true; }

extern SensorConfig configs[NUM_SENSORS];
extern bool needSensorInit;
extern uint32_t nextInit;


#endif //INC_4SENSE_COMMON_H
