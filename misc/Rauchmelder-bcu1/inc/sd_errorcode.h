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

#ifndef SMOKE_DETECTOR_ERRORCODE_H_
#define SMOKE_DETECTOR_ERRORCODE_H_

#include <stdint.h>

class SmokeDetectorGroupObjects;

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

class SmokeDetectorErrorCode
{
public:
    SmokeDetectorErrorCode(const SmokeDetectorGroupObjects *groupObjects);
    ~SmokeDetectorErrorCode() = delete;

    void batteryLow(bool battLow);
    void temperature_1_fault(bool faulty);
    void temperature_2_fault(bool faulty);
    void coverPlateAttached(bool attached);
    void supplyVoltageDisabled(bool disabled);
    void communicationTimeout(bool timedout);

private:
    bool batteryLow() const;
    bool malfunction() const;
    bool isSet(SdErrorCode toCheck) const;
    void set(SdErrorCode error, bool newValue);
    static uint8_t errorCodeToUint8(SdErrorCode code);

    uint8_t errorCode;
    const SmokeDetectorGroupObjects *groupObjects;
};

#endif /* SMOKE_DETECTOR_ERRORCODE_H_ */
