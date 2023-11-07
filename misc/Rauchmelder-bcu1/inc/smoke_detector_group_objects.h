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

#ifndef SMOKE_DETECTOR_GROUP_OBJECTS_H_
#define SMOKE_DETECTOR_GROUP_OBJECTS_H_

#include "rm_const.h"

class ComObjects;

class SmokeDetectorGroupObjects
{
public:
    SmokeDetectorGroupObjects(ComObjects *comObjects);
    ~SmokeDetectorGroupObjects() = default;

public:
    void send(GroupObject groupObject) const;
    void setValue(GroupObject groupObject, uint32_t value) const;
    uint32_t read(GroupObject groupObject) const;
    void write(GroupObject groupObject, uint32_t value) const;
    void writeIfChanged(GroupObject groupObject, uint32_t value) const;

private:
    ComObjects *comObjects;
};

#endif /* SMOKE_DETECTOR_GROUP_OBJECTS_H_ */
