//
// Created by Mario Theodoridis on 17.05.20.
//

#ifndef INC_RAIN_COMMON_H
#define INC_RAIN_COMMON_H

#include <stdint.h>

#ifdef LOGGING
    #include "log/logger.h"
    #define LOG(...) serPrintf(__VA_ARGS__)
#else
    #define LOG(...) // nada!
#endif

#ifndef uint
typedef unsigned int uint;
#endif
#include "params.h"

#endif //INC_RAIN_COMMON_H
