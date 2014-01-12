/*
 *  com_objects.h - EIB Communication objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_com_objects_h
#define sblib_com_objects_h

#include <sblib/eib/bcu.h>
#include <sblib/eib/types.h>

class ComObjects;


/**
 * The communication objects of the device.
 */
extern ComObjects comObjects;


/**
 * Class for the communication objects.
 */
class ComObjects
{
public:
    /**
     * Set the value of a communication object.
     *
     * @param objno - the number of the communication object.
     * @param value - the value to set.
     */
    void set(short objno, int value);

    /**
     * Set the value of a communication object. The number of bytes
     * to be set depends on the type of the communication object.
     *
     * @param objno - the number of the communication object.
     * @param value - the array of value bytes to set.
     */
    void set(short objno, byte* value);

    /**
     * Get the value of a communication object as an integer value.
     *
     * @param objno - the number of the communication object.
     * @return The value of the communication object.
     */
    int asInt(short objno) const;

    /**
     * Get a pointer to the value of a communication object.
     *
     * @param objno - the number of the communication object.
     * @return A pointer to the value of the communication object.
     */
    byte* asArray(short objno) const;

    /**
     * Get the type of a communication object.
     *
     * @param objno - the number of the communication object.
     * @return The type of the communication object.
     */
    ComType type(short objno) const;

    /**
     * Get the size of a communication object in bytes.
     *
     * @param objno - the number of the communication object.
     * @return The size of the communication object.
     */
    short size(short objno) const;

    /**
     * @return The number of communication objects that the device has.
     */
    short count() const;
};

#endif /*sblib_com_objects_h*/
