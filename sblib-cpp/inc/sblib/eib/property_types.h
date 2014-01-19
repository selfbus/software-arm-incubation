/*
 *  property_types.h - BCU 2 property types of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_property_types_h
#define sblib_property_types_h

#include <sblib/types.h>
#include <sblib/utils.h>

// See BCU2 help:
// System Architecture > Interface Objects > User Interface Objects > Attributes of Properties


/**
 * Definition of a property.
 */
struct PropertyDef
{
    byte id;                //!< The ID of the property
    byte control;           //!< Control bits: 7=writable, 6=isArray, 5=isPointer, 4..0=type
    unsigned short valPtr;  //!< Pointer to value (if isPointer==1) or value
                            //!< Bit 15: 0=variable/array pointer, 1=pointer to user function
};

/**
 * Data type of a property.
 */
enum PropertyDataType
{
    PDT_CONTROL = 0,         //!< length: 1 read, 10 write
    PDT_CHAR,                //!< length: 1
    PDT_UNSIGNED_CHAR,       //!< length: 1
    PDT_INT,                 //!< length: 2
    PDT_UNSIGNED_INT,        //!< length: 2
    PDT_EIB_FLOAT,           //!< length: 2
    PDT_DATE,                //!< length: 3
    PDT_TIME,                //!< length: 3
    PDT_LONG,                //!< length: 4
    PDT_UNSIGNED_LONG,       //!< length: 4
    PDT_FLOAT,               //!< length: 4
    PDT_DOUBLE,              //!< length: 8
    PDT_CHAR_BLOCK,          //!< length: 10
    PDT_POLL_GROUP_SETTING,  //!< length: 3
    PDT_SHORT_CHAR_BLOCK,    //!< length: 5
    //---
    PDT_GENERIC_01,          //!< length: 1
    PDT_GENERIC_02,          //!< length: 2
    PDT_GENERIC_03,          //!< length: 3
    PDT_GENERIC_04,          //!< length: 4
    PDT_GENERIC_05,          //!< length: 5
    PDT_GENERIC_06,          //!< length: 6
    PDT_GENERIC_07,          //!< length: 7
    PDT_GENERIC_08,          //!< length: 8
    PDT_GENERIC_09,          //!< length: 9
    PDT_GENERIC_10,          //!< length: 10
};


/**
 * Property ID.
 */
// See KNX 3/7/3 Standardized Identifier Tables, p. 17 for property data types
enum PropertyID
{
    /** Device object property: service control. */
    PID_SERVICE_CONTROL = 8,

    /** Device object property: serial number. */
    PID_SERIAL_NUMBER = 11,

    /** Device object property: manufacturer ID. */
    PID_MANUFACTURER_ID = 12,

    /** Device object property: device control. */
    PID_DEVICE_CONTROL = 14,

    /** Device object property: order info. */
    PID_ORDER_INFO = 15,

    /** Device object property: PEI type. */
    PID_PEI_TYPE = 16,

    /** Device object property: port configuration. */
    PID_PORT_CONFIGURATION = 17,


    /** Application object property: load state control. */
    PID_LOAD_STATE_CONTROL = 5,

    /** Application object property: run state control. */
    PID_RUN_STATE_CONTROL = 6,

    /** Application object property: program version. */
    PID_PROG_VERSION = 13,


    /** Address / association table object property: table address. */
    PID_TABLE_REFERENCE = 7
};


/**
 * Property control constants.
 */
enum PropertyControl
{
    PC_WRITABLE = 0x80,   //!< The property can be modified
    PC_ARRAY = 0x40,      //!< The property is an array (max. 255 bytes)
    PC_POINTER = 0x20,    //!< valPtr of the property definition is a pointer
    PC_TYPE_MASK = 0x1f   //!< Bit mask for the property type
};

/**
 * Property pointer type.
 */
enum PropertyPointerType
{
    PPT_USER_RAM = 0,         //!< Pointer to user RAM
    PPT_USER_EEPROM = 0x1000, //!< Pointer to user EEPROM
    PPT_CONSTANTS = 0x2000,   //!< Pointer to internal constants table
    PPT_MASK = 0x7000,        //!< Bitmask for property pointer types
    PPT_OFFSET_MASK = 0x0fff  //!< Bitmask for property pointer offsets
};

/** Define a PropertyDef pointer to variable v in the userRam */
#define PD_USER_RAM_OFFSET(v) (OFFSET_OF(UserRam, v) + PPT_USER_RAM)

/** Define a PropertyDef pointer to variable v in the userEeprom */
#define PD_USER_EEPROM_OFFSET(v) (OFFSET_OF(UserEeprom, v) + PPT_USER_EEPROM)

/** Define a PropertyDef pointer to variable v in the internal constants table */
#define PD_CONSTANTS_OFFSET(v) (OFFSET_OF(ConstPropValues, v) + PPT_CONSTANTS)


#endif /*sblib_property_types_h*/
