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
    smokeBoxDefect            = 0x10, //!< Smoke box contaminated/defective
    supplyVoltageDisabled     = 0x20, //!< The 12V supply voltage is disabled
    coverplateNotAttached     = 0x40, //!< Smoke detector is not attached to cover plate
};

class SmokeDetectorErrorCode
{
public:
    SmokeDetectorErrorCode();
    ~SmokeDetectorErrorCode() = delete;

    void clearAllErrors();

    bool getBatteryLow() const;
    bool setBatteryLow(bool batteryLow);
    bool setTemperature_1_state(bool faulty);
    bool setTemperature_2_state(bool faulty);
    bool getCoverPlateAttached() const;
    bool setCoverPlateAttached(bool attached);
    bool getSupplyVoltageDisabled() const;
    bool setSupplyVoltageDisabled(bool disabled);
    bool getCommunicationTimeout() const;
    bool setCommunicationTimeout(bool timedout);

    bool getMalfunctionState() const;

    uint8_t getErrorCode() const;

private:
    bool isSet(SdErrorCode toCheck) const;
    bool clearError(SdErrorCode toClear);
    bool setError(SdErrorCode toSet);
    bool wouldChangeErrorState(SdErrorCode errorToCompare, bool set) const;
    static uint8_t errorCodeToUint8(SdErrorCode code);

    uint8_t errorCode;
};



#endif /* SMOKE_DETECTOR_ERRORCODE_H_ */
/** @}*/