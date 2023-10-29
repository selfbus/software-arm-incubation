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

bool SmokeDetectorErrorCode::setError(SdErrorCode error, bool set)
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

    bool errorCodeChanged = (oldErrorCode != errorCode);
    if (errorCodeChanged)
    {
        errorCodeChangedCallback();
    }
    return errorCodeChanged;
}

bool SmokeDetectorErrorCode::batteryLow() const
{
    return isSet(SdErrorCode::batteryLow);
}

bool SmokeDetectorErrorCode::batteryLow(bool batteryLow)
{
    return setError(SdErrorCode::batteryLow, batteryLow);
}

bool SmokeDetectorErrorCode::temperature_1_fault(bool faulty)
{
    return setError(SdErrorCode::temperatureSensor_1_fault, faulty);
}

bool SmokeDetectorErrorCode::temperature_2_fault(bool faulty)
{
    return setError(SdErrorCode::temperatureSensor_2_fault, faulty);
}

bool SmokeDetectorErrorCode::coverPlateAttached() const
{
    return (!isSet(SdErrorCode::coverplateNotAttached));
}

bool SmokeDetectorErrorCode::coverPlateAttached(bool coverPlateAttached)
{
    return setError(SdErrorCode::coverplateNotAttached, !coverPlateAttached);
}

bool SmokeDetectorErrorCode::supplyVoltageDisabled() const
{
    return isSet(SdErrorCode::supplyVoltageDisabled);
}

bool SmokeDetectorErrorCode::supplyVoltageDisabled(bool disabled)
{
    return setError(SdErrorCode::supplyVoltageDisabled, disabled);
}

bool SmokeDetectorErrorCode::communicationTimeout() const
{
    return isSet(SdErrorCode::communicationTimeout);
}

bool SmokeDetectorErrorCode::communicationTimeout(bool timedout)
{
    return setError(SdErrorCode::communicationTimeout, timedout);
}

bool SmokeDetectorErrorCode::malfunctionState() const
{
    bool malFunction = (0 != (errorCode & ~errorCodeToUint8(SdErrorCode::batteryLow)));
    return malFunction;
}

uint8_t SmokeDetectorErrorCode::errorCodeToUint8(SdErrorCode code)
{
    return static_cast<uint8_t>(code);
}


/** @}*/
