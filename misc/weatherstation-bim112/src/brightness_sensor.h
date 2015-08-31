/*
 *  brigthness_sensor.h - The class defining the behavior of one brightness sensor
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef BRIGHTNESS_SENSOR_H_
#define BRIGHTNESS_SENSOR_H_

#include <sblib/eib.h>
#include <sblib/timeout.h>
#include "weatherstation.h"
#include "threshold.h"

class Brightness_Sensor
{

public:
    Brightness_Sensor();
    void Initialize(unsigned int number);

    void periodic();

private:
    // internal state
    unsigned int number;
    unsigned int lastSentBrightness;
    Timeout      cyclicSend;
    // stored config values
    unsigned int cyclicPeriod;
    Threshold threshold[2];
};

#endif /* BRIGHTNESS_SENSOR_H_ */
