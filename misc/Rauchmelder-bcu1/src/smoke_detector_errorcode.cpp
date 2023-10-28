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

SmokeDetectorErrorCode::SmokeDetectorErrorCode()
    : errorCode(static_cast<uint8_t>(SdErrorCode::noError))
{

}

void SmokeDetectorErrorCode::clearAllErrors()
{
    errorCode = errorCodeToUint8(SdErrorCode::noError);
}

bool SmokeDetectorErrorCode::setError(SdErrorCode errorToSet)
{
    if (!wouldChangeErrorState(errorToSet, true))
    {
        return false;
    }
    errorCode = errorCode | errorCodeToUint8(errorToSet);
    return true;
}

bool SmokeDetectorErrorCode::clearError(SdErrorCode errorToClear)
{
    if (!wouldChangeErrorState(errorToClear, false))
    {
        return false;
    }
    errorCode = errorCode & static_cast<uint8_t>(~errorCodeToUint8(errorToClear));
    return true;
}

bool SmokeDetectorErrorCode::isSet(SdErrorCode errorToCheck) const
{
    bool result = (0 != (errorCode & errorCodeToUint8(errorToCheck)));
    return result;
}

uint8_t SmokeDetectorErrorCode::getErrorCode() const
{
    return errorCode;
}

bool SmokeDetectorErrorCode::wouldChangeErrorState(SdErrorCode errorToCompare, bool set) const
{
    uint8_t newErrorCode = errorCode;
    uint8_t compare = errorCodeToUint8(errorToCompare);

    if (set)
    {
        newErrorCode |= compare;
    }
    else
    {
        newErrorCode &= static_cast<uint8_t>(~compare);
    }
    return newErrorCode != errorCode;
}

bool SmokeDetectorErrorCode::getBatteryLow() const
{
    return isSet(SdErrorCode::batteryLow);
}

bool SmokeDetectorErrorCode::setBatteryLow(bool batteryLow)
{
    if (batteryLow)
    {
        return setError(SdErrorCode::batteryLow);
    }
    else
    {
        return clearError(SdErrorCode::batteryLow);
    }
}

bool SmokeDetectorErrorCode::setTemperature_1_state(bool sensorBroken)
{
    if (sensorBroken)
    {
        return setError(SdErrorCode::temperatureSensor_1_fault);
    }
    else
    {
        return clearError(SdErrorCode::temperatureSensor_1_fault);
    }
}

bool SmokeDetectorErrorCode::setTemperature_2_state(bool sensorBroken)
{
    if (sensorBroken)
    {
        return setError(SdErrorCode::temperatureSensor_2_fault);
    }
    else
    {
        return clearError(SdErrorCode::temperatureSensor_2_fault);
    }
}

bool SmokeDetectorErrorCode::getCoverPlateAttached() const
{
    return (!isSet(SdErrorCode::coverplateNotAttached));
}

bool SmokeDetectorErrorCode::setCoverPlateAttached(bool coverPlateAttached)
{
    if (coverPlateAttached)
    {
        return clearError(SdErrorCode::coverplateNotAttached);
    }
    else
    {
        return setError(SdErrorCode::coverplateNotAttached);
    }
}

bool SmokeDetectorErrorCode::getSupplyVoltageDisabled() const
{
    return isSet(SdErrorCode::supplyVoltageDisabled);
}

bool SmokeDetectorErrorCode::setSupplyVoltageDisabled(bool disabled)
{
    if (disabled)
    {
        return setError(SdErrorCode::supplyVoltageDisabled);
    }
    else
    {
        return clearError(SdErrorCode::supplyVoltageDisabled);
    }
}

bool SmokeDetectorErrorCode::getCommunicationTimeout() const
{
    return isSet(SdErrorCode::communicationTimeout);
}

bool SmokeDetectorErrorCode::setCommunicationTimeout(bool timedout)
{
    if (timedout)
    {
        return setError(SdErrorCode::communicationTimeout);
    }
    else
    {
        return clearError(SdErrorCode::communicationTimeout);
    }
}

bool SmokeDetectorErrorCode::getMalfunctionState() const
{
    bool malFunction;
    malFunction = (errorCode & ~errorCodeToUint8(SdErrorCode::batteryLow)) != 0;
    return malFunction;
}

uint8_t SmokeDetectorErrorCode::errorCodeToUint8(SdErrorCode code)
{
    return static_cast<int8_t>(code);
}


/** @}*/
