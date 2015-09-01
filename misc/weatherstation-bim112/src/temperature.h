/*
 *  temperature.h - Handling of the temperature sensor
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include "sensor_base.h"
#include "threshold.h"

class Temperature : SensorBase
{
public:
    Temperature();

    /**
     * Read the configuration from the EEPROM and setup the dusk function.
     */
    void Initialize(void);
    /**
     * Return the last read temperature
     *
     * @return the las read temperature.
     */
    unsigned int current();

    /**
     * Handle the periodic functions of the dusk function.
     */
    void periodic();

protected:
    /**
     * Return the current temperature.
     *
     * @return current temperature
     */
    virtual unsigned int _newValue();

private:
    Threshold threshold[2];

private:
    unsigned int temp;
    unsigned int alignmentOffset;
};

inline unsigned int Temperature::current()
{
    return temp;
}

#endif /* TEMPERATURE_H_ */
