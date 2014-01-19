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
    // Load state control
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(addrTabLoaded) },

    // Pointer to the address table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT, 0x0116 },

    // End of table
    PROPERTY_DEF_TABLE_END
};

//
// The properties of the association table object
//
static const PropertyDef assocTabObjectProps[] =
{
    // Load state control
    { PID_LOAD_STATE_CONTROL, PDT_CONTROL|PC_WRITABLE|PC_POINTER, PD_USER_EEPROM_OFFSET(assocTabLoaded) },

    // Pointer to the association table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT|PC_POINTER, PD_USER_EEPROM_OFFSET(assocTabPtr16) },

    // End of table
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

    // Pointer to the communication objects table
    { PID_TABLE_REFERENCE, PDT_UNSIGNED_INT|PC_POINTER, PD_USER_EEPROM_OFFSET(commsTabPtr16) },

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


bool propertiesValueReadTelegram(int objectIdx, PropertyID propertyId, int count, int start)
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

int loadControl(const PropertyDef* def, byte* valuePtr, const byte* data, int len)
{
    int segmentType, addr, length;
    int state = data[0]; // load state: telegram[12]

    // See KNX 3/5/2, 3.27 DM_LoadStateMachineWrite
    // See KNX 6/6 Profiles, p. 101 for load states
    //
    // State codes 1,2 are incorrect there, see BCU2 help for correct codes:
    // 0: unloaded
    // 1: loaded
    // 2: loading

    // state==3 means write memory. Data:
    // subState addrL addrH data[6]

    switch (state) // the control state
    {
    case 0: // No operation
        return 0; // reply: unloaded

    case 1: // Start loading
        return 2; // reply: loading

    case 2: // Load completed
        *valuePtr = 1;
        return 1; // reply: loaded

    case 3: // Allocate absolute data segment (segment type 0)
        segmentType = data[1];
        addr = makeWord(data[2], data[3]);
        switch (segmentType)
        {
        case 0:  // Allocate absolute data segment (ignored)
            // See KNX 3/5/2 p.84
            break;

        case 1:  // Allocate absolute stack segment (ignored)
            break;

        case 2:  // Allocate absolute task segment
            // data[3+4]: start address (pointer to the table for property #7)
            userEeprom.peiType = data[5];
            userEeprom.manufacturerH = data[6];
            userEeprom.manufacturerL = data[7];
            userEeprom.deviceTypeH = data[8];
            userEeprom.deviceTypeL = data[9];
            userEeprom.version = data[10];
            userEeprom.modified();
            break;

        case 3:  // Task pointer (ignored)
            break;

        case 4:  // Task control 1 (ignored)
            // See KNX 3/5/2 p.85
            break;

        case 5:  // Task control 2
            addr++; // FIXME: remove this dummy debug statement
            // See KNX 3/5/2 p.85
            userEeprom.commsTabPtr16 = makeWord(data[5], data[6]);
            break;

        default:
            IF_DEBUG(fatalError());
            return 3; // reply: error
        }
        return 2; // reply: load complete

    case 4: // Unload
        *valuePtr = 0;
        return 0;   // reply: unloaded
    }

    IF_DEBUG(fatalError());
    return 3; // reply: error
}

bool propertiesValueWriteTelegram(int objectIdx, PropertyID propertyId, int count, int start)
{
    const PropertyDef* def = findProperty(objectIdx, propertyId);
    if (!def) return false;

    if (!(def->control & PC_WRITABLE))
        fatalError();  // shall be replaced with "return false" later

    PropertyDataType type = (PropertyDataType) (def->control & PC_TYPE_MASK);
    const byte* data = bus.telegram + 12;
    int len;

    byte* valuePtr = propertyValuePtr(def);
    if (!valuePtr) return false;

    if (type == PDT_CONTROL)
    {
        len = bus.telegramLen - 13;

//        IF_DEBUG(
//            serial.print("loadControl IDX=");
//            serial.print(objectIdx);
//            serial.print(" PID=");
//            serial.print(propertyId);
//            serial.print(" ");
//            for (int i = 0; i < len; ++i)
//            {
//                if (!i) serial.print(" ");
//                serial.print(data[i], HEX, 2);
//            }
//            serial.println();
//        )

        bcu.sendTelegram[12] = loadControl(def, valuePtr, data, len);
        len = 1;
    }
    else
    {
        len = count * propertySize(type);
        --start;

        memcpy(valuePtr + start * type, data, len);
        memcpy(bcu.sendTelegram + 12, valuePtr + start * type, len);
    }

    bcu.sendTelegram[5] += len;
    return true;
}

bool propertiesDescReadTelegram(int objectIdx, PropertyID propertyId, int propertyIdx)
{
    const PropertyDef* def = findProperty(objectIdx, propertyId);
    if (!def) return false;

    int arrayMaxElems = 1;

    bcu.sendTelegram[11] = (PropertyDataType) (def->control & (PC_TYPE_MASK | PC_WRITABLE));
    bcu.sendTelegram[12] = (arrayMaxElems >> 8) & 15;
    bcu.sendTelegram[13] = arrayMaxElems;
    bcu.sendTelegram[14] = 0x33;

    return true;
}

#endif /* BCU_TYPE == 20 */
