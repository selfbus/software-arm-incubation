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
#include <sblib/internal/variables.h>

#include <string.h>


// Documentation:
// see KNX 6/6 Profiles, p. 94+
// see KNX 3/7/3 Standardized Identifier Tables, p. 11+


const PropertyDef* findProperty(PropertyID propertyId, const PropertyDef* table)
{
    const PropertyDef* def;

    for (def = table; def->id; ++def)
    {
        if (def->id == propertyId)
            return def;
    }

    return 0;
}

/*
 * Get a property definition.
 *
 * @param objectIdx - the index of the interface object.
 * @param propertyId - the property ID.
 * @return The property definition, or 0 if not found.
 */
const PropertyDef* propertyDef(int objectIdx, PropertyID propertyId)
{
    if (objectIdx >= NUM_PROP_OBJECTS)
        return 0;
    return findProperty(propertyId, propertiesTab[objectIdx]);
}

/*
 * Load / configure a property. Called when a "load control" property-write telegram
 * is received.
 *
 * @param objectIdx - the ID of the interface object.
 * @param data - the data bytes
 * @param len - the length of the data.
 * @return The load state.
 */
int loadProperty(int objectIdx, const byte* data, int len)
{
    int segmentType, addr, length;
    int loadState = data[0];

    // See KNX 3/5/2, 3.27 DM_LoadStateMachineWrite
    // See KNX 6/6 Profiles, p. 101 for load states
    // See BCU2 help System Architecture > Load Procedure

    switch (loadState) // the control state
    {
    case 1: // Load
        return 2; // reply: Loading

    case 2: // Load completed
        return 1; // reply: Loaded

    case 3: // Load data: handled below
        break;

    case 4: // Unload
        return 0;  // reply: Unloaded

    default:
        return 3;  // reply: Error
    }

    //
    // Load data (loadState == 3)
    //
    segmentType = data[1];
    addr = makeWord(data[2], data[3]);

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
        if (objectIdx == OT_ADDR_TABLE)
            userEeprom.addrTabAddr = addr;
        else if (objectIdx == OT_ASSOC_TABLE)
            userEeprom.assocTabAddr = addr;
        else if (objectIdx == OT_APPLICATION)
        {
            userEeprom.appPeiType = data[4];
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
        userEeprom.eibObjAddr = addr;
        userEeprom.eibObjCount = data[4];
        break;

    case 5:  // Task control 2
        // data[2+3]: app callback function
        userEeprom.commsTabAddr = makeWord(data[4], data[5]);
        userEeprom.commsSeg0Addr = makeWord(data[6], data[7]);
        userEeprom.commsSeg0Addr = makeWord(data[8], data[9]);
        break;

    default:
        IF_DEBUG(fatalError());
        return 3; // reply: Error
    }

    return 2;
}

bool propertyValueReadTelegram(int objectIdx, PropertyID propertyId, int count, int start)
{
    const PropertyDef* def = propertyDef(objectIdx, propertyId);
    if (!def) return false; // not found

    PropertyDataType type = (PropertyDataType) (def->control & PC_TYPE_MASK);
    byte* valuePtr = def->valuePointer();

    --start;
    int size = def->size();
    int len = count * size;

    if (type < PDT_CHAR_BLOCK)
        reverseCopy(bcu.sendTelegram + 12, valuePtr + start * size, len);
    else memcpy(bcu.sendTelegram + 12, valuePtr + start * size, len);

    bcu.sendTelegram[5] += len;

    return true;
}

bool propertyValueWriteTelegram(int objectIdx, PropertyID propertyId, int count, int start)
{
    const PropertyDef* def = propertyDef(objectIdx, propertyId);
    if (!def) return false; // not found

    if (!(def->control & PC_WRITABLE))
        return false; // not writable

    PropertyDataType type = def->type();
    byte* valuePtr = def->valuePointer();

    const byte* data = bus.telegram + 12;
    int state, len;

    if (type == PDT_CONTROL)
    {
        len = bus.telegramLen - 13;
        state = loadProperty(objectIdx, data, len);
        userEeprom.loadState[objectIdx] = state;
        bcu.sendTelegram[12] = state;
        len = 1;
    }
    else
    {
        len = count * def->size();
        --start;

        reverseCopy(valuePtr + start * type, data, len);
        reverseCopy(bcu.sendTelegram + 12, valuePtr + start * type, len);

        if (def->isEepromPointer())
            userEeprom.modified();
    }

    bcu.sendTelegram[5] += len;
    return true;
}

bool propertyDescReadTelegram(int objectIdx, PropertyID propertyId, int index)
{
    const PropertyDef* def;

    if (propertyId)
        def = propertyDef(objectIdx, propertyId);
    else def = &propertiesTab[objectIdx][index];

    bcu.sendTelegram[10] = index;

    if (!def || !def->id)
    {
        bcu.sendTelegram[9] = propertyId;
        bcu.sendTelegram[11] = 0;
        bcu.sendTelegram[12] = 0;
        bcu.sendTelegram[13] = 0;
        bcu.sendTelegram[14] = 0;
        return false; // not found
    }

    int numElems;
    if ((def->control & PC_ARRAY_POINTER) == PC_ARRAY_POINTER)
        numElems = *def->valuePointer();
    else numElems = 1;

    bcu.sendTelegram[9] = def->id;
    bcu.sendTelegram[11] = def->control & (PC_TYPE_MASK | PC_WRITABLE);
    bcu.sendTelegram[12] = (numElems >> 8) & 15;
    bcu.sendTelegram[13] = numElems;
    bcu.sendTelegram[14] = def->control & PC_WRITABLE ? 0xf1 : 0x50; // wild guess from bus traces

    return true;
}

#endif /*BCU_TYPE >= 20*/
