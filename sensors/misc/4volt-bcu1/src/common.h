//
// Created by Mario Theodoridis on 8.04.21.
//

#ifndef INC_4VOLT_COMMON_H
#define INC_4VOLT_COMMON_H

#include <stdint.h>
#include <sblib/eibBCU1.h>

#define userEeprom (*(bcu.userEeprom))

#ifdef LOGGING
    #include "log/logger.h"
    #define pretty(val) (int)(val*1)
    #define LOG(...) serPrintf(__VA_ARGS__)
#else
    #define LOG(...) // nada!
#endif

// number of seconds before we reset
#define NUM_SENSORS     4
#ifndef uint
typedef unsigned int uint;
#endif

#include "params.h"
#include "Threshold.h"
#include "SensorConfig.h"

void fixRamLoc(COM comNo);
void initSensors();

extern BCU1 bcu;
extern SensorConfig configs[NUM_SENSORS];


#endif //INC_4VOLT_COMMON_H
