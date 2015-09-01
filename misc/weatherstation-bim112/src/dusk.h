/*
 *  dusk.h - Handling of the dusk function
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef DUSK_H_
#define DUSK_H_

#include "sensor_base.h"
#include "threshold.h"

class Dusk : SensorBase
{
public:
    Dusk();

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
     * Return the current dusk value.
     *
     * @return current dusk value
     */
    virtual unsigned int _newValue();

private:
    unsigned int  cycleDayPeriod;
    Timeout       cyclicDaySend;
    Threshold     dayNight;
};

#endif /* DUSK_H_ */
