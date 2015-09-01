/*
 *  sensor_base.h - Base class for sensor measurements
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SENSOR_BASE_H_
#define SENSOR_BASE_H_

#include <sblib/timeout.h>

class SensorBase
{
public:
    SensorBase();
    /**
     * Initialize the sensor
     *
     * @param sendMode
     * @param objNo
     * @param cyclePeriod
     * @param reqChange
     */
    void InitializeSensor(unsigned char sendMode, char objNo, unsigned int cyclePeriod, unsigned int reqChange);

protected:
    /**
     * Return the current value of the sensor.
     *
     * @return current sensor value
     */
    virtual unsigned int _newValue() = 0;

    /**
     * Handle the function related to the sensor value itself.
     *
     * @param newValue - the new sensor value.
     */
    void _handleNewValue(unsigned int newValue);

    /**
     * Convert a config value fronm the EEPROM time milliseconds
     *
     * @param address - the EEPROM address
     * @return the in milliseconds
     */
    unsigned int timeConversionMinSec(unsigned int address);

    unsigned char sendMode;
             char objNo;
    unsigned char reqChange;
    unsigned int  cyclePeriod;

    unsigned int  lastSentValue;
    Timeout       cyclicSend;
};

#endif /* SENSOR_BASE_H_ */
