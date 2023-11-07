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

#ifndef SMOKE_DETECTOR_DEVICE_H_
#define SMOKE_DETECTOR_DEVICE_H_

#include <stdint.h>

#include "rm_const.h"

class SmokeDetectorAlarm;
class SmokeDetectorConfig;
class SmokeDetectorErrorCode;
class SmokeDetectorGroupObjects;

//-----------------------------------------------------------------------------
// Befehle an den Rauchmelder
//-----------------------------------------------------------------------------
enum class Command : uint8_t
{
    rmSerialNumber          = 0,  //!< Gira Command: Seriennummer abfragen
    rmOperatingTime         = 1,  //!< Gira Command: Betriebszeit abfragen
    rmSmokeboxData          = 2,  //!< Gira Command: Rauchkammer Daten abfragen
    rmBatteryAndTemperature = 3,  //!< Gira Command: Batteriespannung und Temperaturen
    rmNumberAlarms_1        = 4,  //!< Gira Command: Anzahl der Alarme #1 abfragen
    rmNumberAlarms_2        = 5,  //!< Gira Command: Anzahl der Alarme #2 abfragen
    rmStatus                = 6,  //!< Gira Command: Status abfragen
};

class SmokeDetectorDevice
{
public:
    SmokeDetectorDevice(const SmokeDetectorConfig *config, const SmokeDetectorGroupObjects *groupObjects, SmokeDetectorAlarm *alarm, SmokeDetectorErrorCode *errorCode);
    ~SmokeDetectorDevice() = delete;

    uint8_t commandTableSize();
    bool hasOngoingMessageExchange();
    void timerEvery500ms();
    bool send_Cmd(Command cmd);
    void recv_bytes();
    void set_alarm_state(RmAlarmState newState);
    void rm_process_msg(uint8_t *bytes, int8_t len);

private:
    void failHardInDebug();

    /**
     * Read raw value from response and convert it to corresponding group object value
     */
    uint32_t readObjectValueFromResponse(uint8_t *rawValue, uint8_t dataType);

    void setSupplyVoltageAndWait(bool enable, uint32_t waitTimeMs);
    void checkRmAttached2BasePlate();

private:
    const SmokeDetectorConfig *config;
    const SmokeDetectorGroupObjects *groupObjects;
    SmokeDetectorAlarm *alarm;
    SmokeDetectorErrorCode *errorCode;
    unsigned char answerWait;          //!< Wenn != 0, dann Zähler für die Zeit die auf eine Antwort vom Rauchmelder gewartet wird.
};

#endif /* SMOKE_DETECTOR_DEVICE_H_ */
