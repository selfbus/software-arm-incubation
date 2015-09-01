/*
 *  threshold.h - The class defining the threshold behavior
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef THRESHOLD_H_
#define THRESHOLD_H_

#include <sblib/timeout.h>

enum
{
    IN_UPPER_LIMIT = 0x01
,   IN_LOWER_LIMIT = 0x02
,   LOWER_LIMIT_TIME_ACTIVE = 0x04
,   UPPER_LIMIT_TIME_ACTIVE = 0x08
};

class Threshold
{
public:
    Threshold();
    unsigned int periodic(unsigned int value);

             int  objNumber;
    unsigned int  lowerLimit;
    unsigned int  upperLimit;
    unsigned int  lowerLimitTime;
    unsigned int  upperLimitTime;
             int  changeLowerLimit;
             int  changeUpperLimit;
    unsigned char sendLimitExceeded;
    unsigned char sendLowerDeviation;
    unsigned int  cycleTime;
    int           blockObjNo;

private:
    unsigned int state;
    Timeout cycleTimeout;
    Timeout limitTimeout;
};

#endif /* THRESHOLD_H_ */
