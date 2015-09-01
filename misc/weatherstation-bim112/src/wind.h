/*
 *  wind.h - Handling of the wind sensor
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef WIND_H_
#define WIND_H_

#include "sensor_base.h"
#include "threshold.h"

class Wind : SensorBase
{
public:
    Wind();

    /**
     * Read the configuration from the EEPROM and setup the dusk function.
     */
    void Initialize(void);

    /**
     * Handle the periodic functions of the dusk function.
     */
    void periodic();

protected:
    /**
     * Return the current wind speed value.
     *
     * @return current wind speed value
     */
    virtual unsigned int _newValue();
private:
    Threshold threshold[2];
};

#endif /* WIND_H_ */
