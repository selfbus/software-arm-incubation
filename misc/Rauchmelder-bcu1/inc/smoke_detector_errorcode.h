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
 * @file   smoke_detector_errorcodes.h
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2023
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef SMOKE_DETECTOR_ERRORCODE_H_
#define SMOKE_DETECTOR_ERRORCODE_H_

#include <stdint.h>

/**
 * Smoke detector module error codes
 */
enum class SdErrorCode : uint8_t
{
    noError                   = 0x00, //!< No error
    batteryLow                = 0x01, //!< Battery low/empty
    communicationTimeout      = 0x02, //!< Smoke detector does not respond
    temperatureSensor_1_fault = 0x04, //!< Temperature sensor 1 faulty
    temperatureSensor_2_fault = 0x08, //!< Temperature sensor 2 faulty
    supplyVoltageDisabled     = 0x10, //!< The 12V supply voltage is disabled
    coverplateNotAttached     = 0x20, //!< Smoke detector is not attached to cover plate
};

class SmokeDetectorErrorCode;

class SmokeDetectorErrorCodeCallback
{
public:
    virtual void errorCodeChanged(const SmokeDetectorErrorCode *errorCode, bool batteryLowChanged, bool malfunctionChanged) const = 0;
};

class SmokeDetectorErrorCode
{
public:
    SmokeDetectorErrorCode(const SmokeDetectorErrorCodeCallback *errorCodeCallback);
    ~SmokeDetectorErrorCode() = delete;

    void clearAllErrors();

    bool batteryLow() const;
    void batteryLow(bool battLow);
    void temperature_1_fault(bool faulty);
    void temperature_2_fault(bool faulty);
    void coverPlateAttached(bool attached);
    void supplyVoltageDisabled(bool disabled);
    void communicationTimeout(bool timedout);

    bool malfunction() const;

    uint8_t code() const;

private:
    bool isSet(SdErrorCode toCheck) const;
    void set(SdErrorCode error, bool set);
    static uint8_t errorCodeToUint8(SdErrorCode code);

    uint8_t errorCode;
    const SmokeDetectorErrorCodeCallback *errorCodeCallback;
};



#endif /* SMOKE_DETECTOR_ERRORCODE_H_ */
/** @}*/
