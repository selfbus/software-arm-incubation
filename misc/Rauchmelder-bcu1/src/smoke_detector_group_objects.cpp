/*  Original written for LPC922:
 *  Copyright (c) 2015-2017 Stefan Haller
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *  Copyright (c) 2020 Stefan Haller
 *
 *  Refactoring and bug fixes:
 *  Copyright (c) 2023 Darthyson <darth@maptrack.de>
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <type_traits>
#include <sblib/eib/com_objects.h>

#include "smoke_detector_group_objects.h"

SmokeDetectorGroupObjects::SmokeDetectorGroupObjects(ComObjects *comObjects)
    : comObjects(comObjects)
{
    for (auto groupObject : AllGroupObjects())
    {
        setValue(groupObject, 0);
    }
}

/**
 * Send any group object onto the bus with the value currently stored.
 *
 * @param groupObject The group object to send
 */
void SmokeDetectorGroupObjects::send(GroupObject groupObject) const
{
    // The communication objects already have the correct value, but there is no method to
    // just mark them for sending. So, just write the same value again.
    auto objno = groupObjectToObjNo(groupObject);
    comObjects->objectWrite(objno, comObjects->objectRead(objno));
}

/**
 * Set any group object to the respective value. Does not send the change onto the bus.
 *
 * @param groupObject The group object to update the stored value of
 * @param value       New value of the group object
 */
void SmokeDetectorGroupObjects::setValue(GroupObject groupObject, uint32_t value) const
{
    auto objno = groupObjectToObjNo(groupObject);
    comObjects->objectSetValue(objno, value);
}

/**
 * Read any group object value.
 *
 * @param groupObject The group object to read the value of
 * @param value       New value of the group object
 */
uint32_t SmokeDetectorGroupObjects::read(GroupObject groupObject) const
{
    auto objno = groupObjectToObjNo(groupObject);
    return comObjects->objectRead(objno);
}

/**
 * Set any group object to the respective value, and send the group object onto the bus.
 *
 * @param groupObject The group object to update the value of
 * @param value       New value of the group object
 */
void SmokeDetectorGroupObjects::write(GroupObject groupObject, uint32_t value) const
{
    auto objno = groupObjectToObjNo(groupObject);
    comObjects->objectWrite(objno, value);
}

/**
 * Set any group object to the respective value, and if the value differs, send the change onto the bus.
 *
 * @param groupObject The group object to update the stored value of
 * @param value       New value of the group object
 */
void SmokeDetectorGroupObjects::writeIfChanged(GroupObject groupObject, uint32_t value) const
{
    auto objno = groupObjectToObjNo(groupObject);
    auto oldValue = comObjects->objectRead(objno);
    if (oldValue != value)
    {
        comObjects->objectWrite(objno, value);
    }
}

/**
 * Converts a @ref GroupObject to its corresponding group object number.
 *
 * @param groupObject The group object to convert.
 */
int SmokeDetectorGroupObjects::groupObjectToObjNo(GroupObject groupObject)
{
    return static_cast<int>(groupObject);
}
