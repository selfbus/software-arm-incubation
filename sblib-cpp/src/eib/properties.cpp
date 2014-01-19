/*
 *  properties.cpp - BCU 2 properties of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#if BCU_TYPE >= 20

#include <sblib/eib/properties.h>

#include <sblib/core.h>
#include <sblib/eib/user_memory.h>
#include <sblib/eib/bcu.h>
#include <sblib/internal/functions.h>

#include <string.h>


// Documentation:
// see KNX 6/6 Profiles, p. 94+
// see KNX 3/7/3 Standardized Identifier Tables, p. 11+


//
// Constant property values
//
typedef struct
{
    unsigned char serial[6];
    unsigned char order[10];
} ConstPropValues;

static const ConstPropValues constPropValues =
{
    /*serial number*/ { 0x11, 0x22, 0x33, 0x44, 0x55, 0x67 },
    /*order number*/  { 0x11, 0x12, 0x00, 0x00, 0x22, 0x23, 0x00, 0x00, 0x33, 0x34 }
};


//
// The properties of the device object
//
static const PropertyDef deviceObjectProps[] =
{
    // Service control: 2 bytes stored in userEeprom.serviceControl
    { PID_SERVICE_CONTROL, PDT_UNSIGNED_INT|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(serviceControl) },

    // Serial number: 6 byte data, stored in userEeprom.serial
    { PID_SERIAL_NUMBER, PDT_GENERIC_06|PC_POINTER, PD_USER_EEPROM_OFFSET(serial) },

    // Manufacturer ID: unsigned int, stored in userEeprom.manufacturerH (and manufacturerL)
    { PID_MANUFACTURER_ID, PDT_UNSIGNED_INT|PC_POINTER, PD_USER_EEPROM_OFFSET(manufacturerH) },

    // Device control:
    { PID_DEVICE_CONTROL, PDT_GENERIC_01|PC_WRITABLE|PC_POINTER, PD_USER_RAM_OFFSET(deviceControl) },

    // Order number: 10 byte data, stored in constPropValues
    { PID_ORDER_INFO, PDT_GENERIC_10|PC_POINTER, PD_CONSTANTS_OFFSET(order) },

    // PEI type: 1 byte, stored here
    { PID_PEI_TYPE, PDT_UNSIGNED_CHAR, 0x02 },

    // Port configuration: 1 byte, stored in userEeprom.>portADDR
    { PID_PORT_CONFIGURATION, PDT_UNSIGNED_CHAR|PC_POINTER, PD_USER_EEPROM_OFFSET(portADDR) },

    // End of table
    PROPERTY_DEF_TABLE_END
};

//
// The properties of the address table object
//
static const PropertyDef addrTabObjectProps[] =
{
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(addrTabLoaded) },
    PROPERTY_DEF_TABLE_END
};

//
// The properties of the association table object
//
static const PropertyDef assocTabObjectProps[] =
{
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(assocTabLoaded) },
    PROPERTY_DEF_TABLE_END
};

//
// The properties of the application program object
//
static const PropertyDef appObjectProps[] =
{
    // Load state control
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(appLoaded) },

    // Run state control
    { PID_RUN_STATE_CONTROL, PDT_CONTROL|PC_POINTER, PD_USER_EEPROM_OFFSET(appRunning) },

    // Program version
    { PID_PROG_VERSION, PDT_GENERIC_05|PC_POINTER, PD_USER_EEPROM_OFFSET(manufacturerH) },

    // End of table
    PROPERTY_DEF_TABLE_END
};


#define PROP_OBJS_SIZE 4

//
// The interface objects
//
static const PropertyDef* propObjsTab[PROP_OBJS_SIZE] =
{
    deviceObjectProps,    // Object 0
    addrTabObjectProps,   // Object 1
    assocTabObjectProps,  // Object 2
    appObjectProps        // Object 3
};

// The property sizes in bytes
static const byte propertySizes[] =
{
    1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 8, 10, 3, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};


/*
 * Get the size of a propery value in bytes.
 *
 * @param type - the property type: PDT_CHAR, PDT_INT, ...
 * @return The size in bytes.
 */
inline int propertySize(PropertyDataType type)
{
    return propertySizes[type];
}

const PropertyDef* findProperty(const PropertyDef* tab, PropertyID propertyId)
{
    const PropertyDef* def;

    for (def = tab; def->id; ++def)
    {
        if (def->id == propertyId)
            return def;
    }

    return 0;
}

/*
 * Find the property definition of an object's property.
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @return The property definition, or 0 if not found.
 */
const PropertyDef* findProperty(int objectIdx, PropertyID propertyId)
{
    if (objectIdx < PROP_OBJS_SIZE)
        return findProperty(propObjsTab[objectIdx], propertyId);
    return 0;
}

/*
 * Get the value pointer from the property definition.
 *
 * @param def - the property definition.
 *
 * @return The pointer to the property value.
 */
static byte* propertyValuePtr(const PropertyDef* def)
{
    byte* valuePtr = 0;

    if (def->control & PC_POINTER)
    {
        int offs = def->valPtr & PPT_OFFSET_MASK;
        switch (def->valPtr & PPT_MASK)
        {
        case PPT_USER_RAM:
            return userRamData + offs;
        case PPT_USER_EEPROM:
            return userEepromData + offs;
        case PPT_CONSTANTS:
            return ((byte*) &constPropValues) + offs;
        default:
            fatalError(); // invalid property pointer type encountered
            break;
        }
    }

    return (byte*) &def->valPtr;
}


bool propertiesReadTelegram(int objectIdx, PropertyID propertyId, int count, int start)
{
    const PropertyDef* def = findProperty(objectIdx, propertyId);
    if (!def) fatalError();  // shall be replaced with "return false" later

    PropertyDataType type = (PropertyDataType) (def->control & PC_TYPE_MASK);
    byte* valuePtr = propertyValuePtr(def);
    if (!valuePtr) fatalError();  // shall be replaced with "return false" later

    --start;
    int len = count * propertySize(type);

    memcpy(bcu.sendTelegram + 12, valuePtr + start * type, len);
    bcu.sendTelegram[5] += len;

    return true;
}

bool propertiesWriteTelegram(int objectIdx, PropertyID propertyId, int count, int start)
{
    const PropertyDef* def = findProperty(objectIdx, propertyId);
    if (!def) return false;

    if (!(def->control & PC_WRITABLE))
        fatalError();  // shall be replaced with "return false" later

    PropertyDataType type = (PropertyDataType) (def->control & PC_TYPE_MASK);
    int len = count * propertySize(type);
    byte* valuePtr = propertyValuePtr(def);
    if (!valuePtr) return false;

    --start;

    if (type == PDT_CONTROL)
    {
        // See KNX 6/6 Profiles, p. 101 for load states
        // State codes 1,2 are incorrect there, see BCU2 help for correct codes:
        // 0: unloaded
        // 1: loaded
        // 2: loading

        int state = bus.telegram[12];

        // state==3 means write memory. Data:
        // subState addrL addrH data[6]

        switch (bus.telegram[12]) // the state
        {
        case 0: // unloaded
            break;
        case 1: // loaded
            break;
        case 2: // loading
            *valuePtr = 1;
            state = 1; // reply:  loaded
            break;
        case 4:
            *valuePtr = 0;
            state = 0;  // reply: unloaded
            break;
        default:
            *valuePtr = 1;
            state = 2;  // reply: loading
            break;
        }

        bcu.sendTelegram[12] = state;
        len = 1;
    }
    else
    {
        memcpy(valuePtr + start * type, bus.telegram + 12, len);
        memcpy(bcu.sendTelegram + 12, valuePtr + start * type, len);
    }

    bcu.sendTelegram[5] += len;
    return true;
}

#endif /* BCU_TYPE == 20 */
