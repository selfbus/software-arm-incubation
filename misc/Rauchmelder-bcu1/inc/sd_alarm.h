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

#ifndef SMOKE_DETECTOR_ALARM_H_
#define SMOKE_DETECTOR_ALARM_H_

#include <stdint.h>

#include "sd_shared_enums.h"

class SmokeDetectorConfig;
class SmokeDetectorGroupObjects;

class SmokeDetectorAlarm
{
public:
    SmokeDetectorAlarm(const SmokeDetectorConfig *config, const SmokeDetectorGroupObjects *groupObjects);
    ~SmokeDetectorAlarm() = delete;

    bool hasAlarm() const;
    void groupObjectUpdated(GroupObject groupObject);
    void deviceStatusUpdate(bool newAlarmLocal, bool newTestAlarmLocal, bool newAlarmFromBus, bool newTestAlarmFromBus);
    void deviceButtonPressed();
    RmAlarmState loopCheckAlarmState();
    void timerEverySecond();
    void timerEveryMinute();

private:
    void resetBusAlarm();
    void sendAlarmNetwork();
    void sendAlarmStatus();
    void sendAlarmReset() const;
    void sendTestAlarmNetwork() const;
    void sendTestAlarmStatus();
    void setDelayedAlarmCounter(uint8_t newValue);

private:
    const SmokeDetectorConfig *config;
    const SmokeDetectorGroupObjects *groupObjects;
    bool deviceHasAlarmLocal;     //!< Device has an alarm due to smoke, temperature, or wired trigger (green networking terminal)
    bool deviceHasAlarmBus;       //!< Device has an alarm due to bus trigger (via KNX)
    bool deviceHasTestAlarmLocal; //!< Device has a test alarm due to button press or wired trigger
    bool deviceHasTestAlarmBus;   //!< Device has a test alarm due to bus trigger (via KNX)
    bool requestedAlarmBus;       //!< Desired alarm state per KNX bus
    bool requestedTestAlarmBus;   //!< Desired test alarm state per KNX bus
    bool ignoreBusAlarm;          //!< After Alarm Reset via KNX bus, ignore bus alarms for some time

    // Although there is a single alarm interval in ETS, AlarmNetwork and AlarmStatus can be needed to be sent
    // at different times. This can happen when a delayed alarm is configured: Then, AlarmStatus is sent
    // immediately and AlarmNetwork is only sent after the delay is over. From these respective points in time
    // on these two group objects are then sent periodically.
    uint8_t alarmNetworkCounter;  //!< Countdown to next periodic sending of AlarmNetwork in seconds
    uint8_t alarmStatusCounter;   //!< Countdown to next periodic sending of AlarmStatus in seconds
    uint8_t testAlarmCounter;     //!< Countdown to next periodic sending of TestAlarmNetwork and TestAlarmStatus in seconds
    uint8_t delayedAlarmCounter;  //!< Countdown to delayed alarm sending in seconds
};

#endif /* SMOKE_DETECTOR_ALARM_H_ */
