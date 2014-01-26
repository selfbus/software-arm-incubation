/*
 *  property_defs.cpp - definitions of BCU 2 properties of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#if BCU_TYPE >= 20

#include <sblib/version.h>
#include <sblib/eib/user_memory.h>
#include <sblib/internal/variables.h>


//
// The properties of the device object
// See BCU2 help
//
static const PropertyDef deviceObjectProps[] =
{
    // Interface object type: 2 bytes
    { PID_OBJECT_TYPE, PDT_UNSIGNED_INT, 0x0000 },

    // Device control
    { PID_DEVICE_CONTROL, PDT_GENERIC_01|PC_WRITABLE|PC_POINTER, PD_USER_RAM_OFFSET(deviceControl) },

    // Load state control
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(loadState[OT_DEVICE]) },

    // Service control: 2 bytes stored in userEeprom.serviceControl
    { PID_SERVICE_CONTROL, PDT_UNSIGNED_INT|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(serviceControl) },

    // Firmware revision: 1 byte
    { PID_FIRMWARE_REVISION, PDT_UNSIGNED_CHAR, SBLIB_VERSION },

    // Serial number: 6 byte data, stored in userEeprom.serial
    { PID_SERIAL_NUMBER, PDT_GENERIC_06|PC_POINTER, PD_USER_EEPROM_OFFSET(serial) },

    // Manufacturer ID: unsigned int, stored in userEeprom.manufacturerH (and manufacturerL)
    { PID_MANUFACTURER_ID, PDT_UNSIGNED_INT|PC_POINTER, PD_USER_EEPROM_OFFSET(manufacturerH) },

    // Order number: 10 byte data, stored in userEeprom.serial
    // Ok this is a hack. The serial number and the following 4 bytes are returned.
    { PID_ORDER_INFO, PDT_GENERIC_10|PC_POINTER, PD_USER_EEPROM_OFFSET(serial) },

    // PEI type: 1 byte
    { PID_PEI_TYPE, PDT_UNSIGNED_CHAR|PC_POINTER, PD_USER_RAM_OFFSET(peiType) },

    // Port A configuration: 1 byte, stored in userEeprom.portADDR
    { PID_PORT_CONFIGURATION, PDT_UNSIGNED_CHAR|PC_POINTER, PD_USER_EEPROM_OFFSET(portADDR) },

    // End of table
    PROPERTY_DEF_TABLE_END
};

//
// The properties of the address table object
//
static const PropertyDef addrTabObjectProps[] =
{
    // Interface object type: 2 bytes
    { PID_OBJECT_TYPE, PDT_UNSIGNED_INT, 0x0100 },

    // Load state control
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(loadState[OT_ADDR_TABLE]) },

    // Pointer to the address table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT|PC_ARRAY_POINTER, PD_USER_EEPROM_OFFSET(addrTabAddr) },

    // End of table
    PROPERTY_DEF_TABLE_END
};

//
// The properties of the association table object
//
static const PropertyDef assocTabObjectProps[] =
{
    // Interface object type: 2 bytes
    { PID_OBJECT_TYPE, PDT_UNSIGNED_INT, 0x0200 },

    // Load state control
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(loadState[OT_ASSOC_TABLE]) },

    // Pointer to the association table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT|PC_ARRAY_POINTER, PD_USER_EEPROM_OFFSET(assocTabAddr) },

    // End of table
    PROPERTY_DEF_TABLE_END
};

//
// The properties of the application program object
//
static const PropertyDef appObjectProps[] =
{
    // Interface object type: 2 bytes
    { PID_OBJECT_TYPE, PDT_UNSIGNED_INT, 0x0300 },

    // Load state control
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(loadState[OT_APPLICATION]) },

    // Run state control
    { PID_RUN_STATE_CONTROL, PDT_UNSIGNED_CHAR|PC_POINTER, PD_USER_RAM_OFFSET(runState) },

    // Program version
    { PID_PROG_VERSION, PDT_GENERIC_05|PC_POINTER, PD_USER_EEPROM_OFFSET(manufacturerH) },

    // Pointer to the communication objects table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT|PC_ARRAY_POINTER, PD_USER_EEPROM_OFFSET(commsTabAddr) },

    // End of table
    PROPERTY_DEF_TABLE_END
};


//
// The interface objects
//
const PropertyDef* const propertiesTab[NUM_PROP_OBJECTS] =
{
    deviceObjectProps,    // Object 0
    addrTabObjectProps,   // Object 1
    assocTabObjectProps,  // Object 2
    appObjectProps        // Object 3
};

#endif /*BCU_TYPE >= 20*/
