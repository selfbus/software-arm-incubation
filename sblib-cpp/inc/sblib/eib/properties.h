/*
 *  properties.h - BCU 2 properties of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_properties_h
#define sblib_properties_h

#include <sblib/types.h>

/**
 * Definition of a property.
 */
struct PropertyDef
{
    byte id;                //!< The ID of the property
    byte control;           //!< Control bits: 7=writable, 6=isArray, 5=isPointer, 0..4=type
    unsigned short valPtr;  //!< Pointer to value (if isPointer==1) or value
                            //!< Bit 15: 0=variable/array pointer, 1=pointer to user function
};

/**
 * Find a property definition in a properties table.
 *
 * @param id - the ID of the property to find.
 * @param table - the properties table.
 *
 * @return Pointer to the property definition, 0 if not found.
 */
PropertyDef* findProperty(byte id, PropertyDef* table);


// See BCU2 help:
// System Architecture > Interface Objects > User Interface Objects > Attributes of Properties

/**
 * Type of a property.
 */
enum PropertyType
{
    PT_CONTROL = 0,         //!< length: 1 read, 10 write
    PT_CHAR,                //!< length: 1
    PT_UNSIGNED_CHAR,       //!< length: 1
    PT_INT,                 //!< length: 2
    PT_UNSIGNED_INT,        //!< length: 2
    PT_EIB_FLOAT,           //!< length: 2
    PT_DATE,                //!< length: 3
    PT_TIME,                //!< length: 3
    PT_LONG,                //!< length: 4
    PT_UNSIGNED_LONG,       //!< length: 4
    PT_FLOAT,               //!< length: 4
    PT_DOUBLE,              //!< length: 8
    PT_CHAR_BLOCK,          //!< length: 10
    PT_POLL_GROUP_SETTING,  //!< length: 3
    PT_SHORT_CHAR_BLOCK,    //!< length: 5
    //---
    PT_GENERIC01,           //!< length: 1
    PT_GENERIC02,           //!< length: 2
    PT_GENERIC03,           //!< length: 3
    PT_GENERIC04,           //!< length: 4
    PT_GENERIC05,           //!< length: 5
    PT_GENERIC06,           //!< length: 6
    PT_GENERIC07,           //!< length: 7
    PT_GENERIC08,           //!< length: 8
    PT_GENERIC09,           //!< length: 9
    PT_GENERIC10,           //!< length: 10
};

#endif /*sblib_properties_h*/
