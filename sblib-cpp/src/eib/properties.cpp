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
#include <sblib/eib/interface_object.h>
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
    unsigned char order[10];
} ConstPropValues;

static const ConstPropValues constPropValues =
{
    /*order number*/  { 0x11, 0x12, 0x00, 0x00, 0x22, 0x23, 0x00, 0x00, 0x33, 0x34 }
};


const PropertyDef* propertyDef(int objectIdx, PropertyID propertyId)
{
    const InterfaceObject* obj = userEeprom.interfaceObject(objectIdx);
    if (obj) return obj->findById(propertyId);
    return 0;
}

bool propertyValueReadTelegram(int objectIdx, PropertyID propertyId, int count, int start)
{
    const PropertyDef* def = propertyDef(objectIdx, propertyId);
    if (!def) return false; // not found

    PropertyDataType type = (PropertyDataType) (def->control & PC_TYPE_MASK);
    byte* valuePtr = def->valuePointer();
    if (!valuePtr) fatalError();  // shall be replaced with "return false" later

    --start;
    int size = def->size();
    int len = count * size;

    memcpy(bcu.sendTelegram + 12, valuePtr + start * size, len);
    bcu.sendTelegram[5] += len;

    return true;
}

bool propertyValueWriteTelegram(int objectIdx, PropertyID propertyId, int count, int start)
{
    InterfaceObject* obj = userEeprom.interfaceObject(objectIdx);
    if (!obj) return false;

    const PropertyDef* def = obj->findById(propertyId);
    if (!def) return false;

    if (!(def->control & PC_WRITABLE))
        return false; // not writable

    PropertyDataType type = def->type();
    const byte* data = bus.telegram + 12;
    int status, len;

    byte* valuePtr = def->valuePointer();
    if (!valuePtr) fatalError(); // should not happen

    if (type == PDT_CONTROL)
    {
        len = bus.telegramLen - 13;
        bcu.sendTelegram[12] = obj->load(data, len);
        len = 1;
    }
    else
    {
        len = count * def->size();
        --start;

        memcpy(valuePtr + start * type, data, len);
        memcpy(bcu.sendTelegram + 12, valuePtr + start * type, len);

        if (def->isEepromPointer())
            userEeprom.modified();
    }

    bcu.sendTelegram[5] += len;
    return true;
}

bool propertyDescReadTelegram(int objectIdx, PropertyID propertyId, int index)
{
    const PropertyDef* def;

    const InterfaceObject* obj = userEeprom.interfaceObject(objectIdx);
    if (!obj)
        def = 0;
    if (propertyId)
        def = obj->findById(propertyId);
    else def = obj->findByIndex(index);

    bcu.sendTelegram[10] = index;

    if (!def)
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

#endif /* BCU_TYPE == 20 */
