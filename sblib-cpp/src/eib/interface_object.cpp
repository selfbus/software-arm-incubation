/*
 *  interface_object.cpp - BCU2 interface object.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#if BCU_TYPE >= 20

#include <sblib/eib/interface_object.h>

#include <sblib/core.h>
#include <sblib/eib/user_memory.h>


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
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(deviceObject.state) },

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

    // PEI type: 1 byte, stored here
    { PID_PEI_TYPE, PDT_UNSIGNED_CHAR, 0x02 },

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
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(addrObject.state) },

    // Pointer to the address table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT|PC_ARRAY_POINTER, PD_USER_EEPROM_OFFSET(addrObject.tableAddr) },

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
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(assocObject.state) },

    // Pointer to the association table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT|PC_ARRAY_POINTER, PD_USER_EEPROM_OFFSET(assocObject.tableAddr) },

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
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(appObject.state) },

    // Run state control
    { PID_RUN_STATE_CONTROL, PDT_UNSIGNED_CHAR|PC_POINTER, PD_USER_RAM_OFFSET(runState) },

    // Program version
    { PID_PROG_VERSION, PDT_GENERIC_05|PC_POINTER, PD_USER_EEPROM_OFFSET(manufacturerH) },

    // Pointer to the communication objects table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT|PC_ARRAY_POINTER, PD_USER_EEPROM_OFFSET(appObject.tableAddr) },

    // End of table
    PROPERTY_DEF_TABLE_END
};

#define PROP_OBJS_SIZE 4

//
// The interface objects
//
static const PropertyDef* const propertiesTab[PROP_OBJS_SIZE] =
{
    deviceObjectProps,    // Object 0
    addrTabObjectProps,   // Object 1
    assocTabObjectProps,  // Object 2
    appObjectProps        // Object 3
};


const PropertyDef* InterfaceObject::findByIndex(int index) const
{
    const PropertyDef* def = &propertiesTab[id][index];
    if (def->id) return def;
    return 0;
}

const PropertyDef* InterfaceObject::findById(int propertyId) const
{
    const PropertyDef* def;

    for (def = propertiesTab[id]; def->id; ++def)
    {
        if (def->id == propertyId)
            return def;
    }

    return 0;
}

byte* InterfaceObject::tablePointer() const
{
    if (tableAddr >= USER_EEPROM_START && tableAddr < USER_EEPROM_END)
        return userEepromData + tableAddr - USER_EEPROM_START;
    if (tableAddr >= USER_RAM_START && tableAddr < USER_RAM_END)
        return userRamData + tableAddr - USER_RAM_START;
    return 0;
}

int InterfaceObject::load(const byte* data, int len)
{
    int segmentType, addr, length;
    int loadState = data[0];

    // See KNX 3/5/2, 3.27 DM_LoadStateMachineWrite
    // See KNX 6/6 Profiles, p. 101 for load states
    // See BCU2 help System Architecture > Load Procedure

    switch (loadState) // the control state
    {
    case 0: // No operation
        state = 0; // reply: Unloaded
        break;

    case 1: // Load
        state = 2; // reply: Loading
        break;

    case 2: // Load completed
        state = 1; // reply: Loaded
        break;

    case 3: // Load data
        segmentType = data[1];
        switch (segmentType)
        {
        case 0:  // Allocate absolute code/data segment (ignored)
            // See KNX 3/5/2 p.84
            // data[2..3]: address
            // data[4..5]: length
            // data[6]: read/write access level
            // data[7]: memory type (1=Low RAM, 2=RAM, 3=EEPROM)
            // data[8]: bit 7: enable checksum control
            break;

        case 1:  // Allocate absolute stack segment (ignored)
            break;

        case 2:  // Segment control record
            // addr is used for addr tab, assoc tab, app main()
            tableAddr = makeWord(data[2], data[3]);
            if (id == IOBJ_APPLICATION)
            {
                userEeprom.peiType = data[4];
                userEeprom.manufacturerH = data[5];
                userEeprom.manufacturerL = data[6];
                userEeprom.deviceTypeH = data[7];
                userEeprom.deviceTypeL = data[8];
                userEeprom.version = data[9];
            }
            userEeprom.modified();
            break;

        case 3:  // Task pointer (ignored)
            // data[2+3]: app init function
            // data[4+5]: app save function
            // data[6+7]: custom PEI handler function
            break;

        case 4:  // Task control 1 (ignored)
            // See KNX 3/5/2 p.85
            // data[2+3]: address of user EIB-objects table
            // data[4]: number of user EIB objects
            break;

        case 5:  // Task control 2
            // data[2+3]: app callback function
            // data[4+5]: communication object table
            // data[6+7]: segment 0 pointer to communication objects
            // data[8+9]: segment 1 pointer to communication objects
            addr = makeWord(data[4], data[5]);
            if (addr) tableAddr = addr;
            userEeprom.segment0addr = makeWord(data[6], data[7]);
            userEeprom.segment1addr = makeWord(data[8], data[9]);
            break;

        default:
            IF_DEBUG(fatalError());
            return 3; // reply: Error
        }
        state = 2; // reply: Loading
        break;

    case 4: // Unload
        state = 0;  // reply: Unloaded
        break;
    }

    return state;
}

#endif /*BCU_TYPE >= 20*/
