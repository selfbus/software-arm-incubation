/*
 *  interface_object.h - BCU2 interface object.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef interface_object_h
#define interface_object_h

#include <sblib/eib/property_types.h>


/**
 * BCU2 interface object ID
 */
enum InterfaceObjectID
{
    /** Device object. */
    IOBJ_DEVICE = 0,

    /** Address table object. */
    IOBJ_ADDR_TABLE = 1,

    /** Association table object. */
    IOBJ_ASSOC_TABLE = 2,

    /** Application program object. */
    IOBJ_APPLICATION = 3
};


/**
 * A BCU2 interface object.
 *
 * The interface object contains properties that allow ETS or any other PC client to access
 * The settings of a device in a hardware independent manner.
 */
class InterfaceObject
{
    // Note for programmers: this class must not have a constructor / destructor
    // and must not contain virtual functions.

public:
    /**
     * The ID of the interface object, see enum InterfaceObjectID.
     */
    byte id;

    /**
     * The object's state for load control:
     *
     * 0 - unloaded
     * 1 - loaded
     * 2 - load in progress
     * 3 - error during load
     */
    byte state;

    /**
     * The 16 bit address of the table. Zero if the object has no data table.
     * The pointer points into BCU address space: into userRam or userEeprom.
     *
     * Use table() to get a real pointer to the table.
     */
    short tableAddr;

    /*
     * Reserved for later use.
     */
    byte reserved[4];

    /**
     * Get a pointer to the table that is addressed by tableAddr.
     *
     * @return Pointer to the table, or 0 if tableAddr is 0.
     */
    byte* tablePointer() const;

    /**
     * Get the definition of a property by specifying the property ID.
     *
     * @param id - the ID of the property
     * @return The property definition, or 0 if not found.
     */
    const PropertyDef* findById(int id) const;

    /**
     * Get the definition of a property by specifying the index into the object's
     * properties table.
     *
     * @param index - the index of the property
     * @return The property definition, or 0 if not found.
     */
    const PropertyDef* findByIndex(int index) const;

    /**
     * Load / configure a property. Usually called when a "load control" property write telegram
     * is received.
     *
     * @param data - the data bytes
     * @param len - the length of data.
     * @return The load status.
     */
    int load(const byte* data, int len);
};


#endif /*interface_object_h*/
