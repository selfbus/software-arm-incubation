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
#include "sensor_base.h"
#include "weatherstation.h"
#include "threshold.h"
#include "sensor_base.h"

class BrightnessSensor : SensorBase
{
public:
    BrightnessSensor();

    /**
     * Read the configuration from the EEPROM and setup the brightness function.
     *
     * @param number - which channel (0 .. EST, 1 .. SOUTH, 2 .. WEST)
     */
    void Initialize(unsigned int number);

    /**
     * Handle the periodic functions of the brightness channel.
     */
    void periodic(unsigned int temperature);

    /**
     * Handle the reaction to an object update from outside.
     */
    void objectUpdated(int number);

private:
    /**
     * Return the current brightness.
     *
     * @return current brightness value
     */
    virtual unsigned int _newValue();

    /**
     * Handle the function related to the facade control.
     *
     * @param newBrightness - the new brightness value.
     */
    void _handleFacade(unsigned int newBrightness, unsigned int temperature);

    // internal state
    unsigned int number;
    unsigned int objNoOffset;

    Threshold threshold[2];
};

#endif /* BRIGHTNESS_SENSOR_H_ */
