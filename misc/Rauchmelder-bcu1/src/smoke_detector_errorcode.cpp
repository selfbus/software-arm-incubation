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

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "smoke_detector_errorcode.h"
#include "smoke_detector_group_objects.h"

SmokeDetectorErrorCode::SmokeDetectorErrorCode(const SmokeDetectorGroupObjects *groupObjects)
    : errorCode(errorCodeToUint8(SdErrorCode::noError)),
      groupObjects(groupObjects)
{
}

void SmokeDetectorErrorCode::batteryLow(bool batteryLow)
{
    set(SdErrorCode::batteryLow, batteryLow);
}

void SmokeDetectorErrorCode::temperature_1_fault(bool faulty)
{
    set(SdErrorCode::temperatureSensor_1_fault, faulty);
}

void SmokeDetectorErrorCode::temperature_2_fault(bool faulty)
{
    set(SdErrorCode::temperatureSensor_2_fault, faulty);
}

void SmokeDetectorErrorCode::coverPlateAttached(bool coverPlateAttached)
{
    set(SdErrorCode::coverplateNotAttached, !coverPlateAttached);
}

void SmokeDetectorErrorCode::supplyVoltageDisabled(bool disabled)
{
    set(SdErrorCode::supplyVoltageDisabled, disabled);
}

void SmokeDetectorErrorCode::communicationTimeout(bool timedout)
{
    set(SdErrorCode::communicationTimeout, timedout);
}

bool SmokeDetectorErrorCode::batteryLow() const
{
    return isSet(SdErrorCode::batteryLow);
}

bool SmokeDetectorErrorCode::malfunction() const
{
    bool malFunction = (0 != (errorCode & ~errorCodeToUint8(SdErrorCode::batteryLow)));
    return malFunction;
}

bool SmokeDetectorErrorCode::isSet(SdErrorCode errorToCheck) const
{
    bool result = (0 != (errorCode & errorCodeToUint8(errorToCheck)));
    return result;
}

void SmokeDetectorErrorCode::set(SdErrorCode error, bool newValue)
{
    if (newValue)
    {
        errorCode |= errorCodeToUint8(error);
    }
    else
    {
        errorCode &= ~errorCodeToUint8(error);
    }

    groupObjects->writeIfChanged(GroupObject::grpObjErrorCode, errorCode);
    groupObjects->writeIfChanged(GroupObject::grpObjBatteryLow, batteryLow());
    groupObjects->writeIfChanged(GroupObject::grpObjMalfunction, malfunction());
}

uint8_t SmokeDetectorErrorCode::errorCodeToUint8(SdErrorCode code)
{
    return static_cast<uint8_t>(code);
}
