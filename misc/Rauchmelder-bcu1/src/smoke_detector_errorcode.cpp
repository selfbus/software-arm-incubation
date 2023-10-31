/**************************************************************************//**
 * @addtogroup SBAPP_MAIN_GROUP Selfbus Applications
 * @defgroup SBAPP_SUB_GROUP_1 Smoke detector KNX module
 * @ingroup SBAPP_MAIN_GROUP
 * @brief   
 * @details 
 *
 *
 * @{
 *
 * @file   smoke_detector_errorcodes.cpp
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2023
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <smoke_detector_errorcode.h>

SmokeDetectorErrorCode::SmokeDetectorErrorCode(const errorCodeChangedCallbackPtr errorCodeChangedCallback)
    : errorCode(errorCodeToUint8(SdErrorCode::noError)),
      errorCodeChangedCallback(errorCodeChangedCallback)
{
}

void SmokeDetectorErrorCode::clearAllErrors()
{
    errorCode = errorCodeToUint8(SdErrorCode::noError);
}

bool SmokeDetectorErrorCode::isSet(SdErrorCode errorToCheck) const
{
    bool result = (0 != (errorCode & errorCodeToUint8(errorToCheck)));
    return result;
}

uint8_t SmokeDetectorErrorCode::code() const
{
    return errorCode;
}

void SmokeDetectorErrorCode::set(SdErrorCode error, bool set)
{
    uint8_t oldErrorCode = errorCode;

    if (set)
    {
        errorCode |= errorCodeToUint8(error);
    }
    else
    {
        errorCode &= ~errorCodeToUint8(error);
    }

    if (oldErrorCode != errorCode)
    {
        errorCodeChangedCallback(error == SdErrorCode::batteryLow, error != SdErrorCode::batteryLow);
    }
}

bool SmokeDetectorErrorCode::batteryLow() const
{
    return isSet(SdErrorCode::batteryLow);
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

bool SmokeDetectorErrorCode::malfunction() const
{
    bool malFunction = (0 != (errorCode & ~errorCodeToUint8(SdErrorCode::batteryLow)));
    return malFunction;
}

uint8_t SmokeDetectorErrorCode::errorCodeToUint8(SdErrorCode code)
{
    return static_cast<uint8_t>(code);
}


/** @}*/
