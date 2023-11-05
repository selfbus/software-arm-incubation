/*
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef SMOKE_DETECTOR_ALARM_H_
#define SMOKE_DETECTOR_ALARM_H_

#include <stdint.h>

#include "smoke_detector_config.h"
#include "smoke_detector_group_objects.h"

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
    const SmokeDetectorConfig *config;
    const SmokeDetectorGroupObjects *groupObjects;
    bool deviceHasAlarmLocal;     //!< Device has an alarm due to smoke, temperature, or wired trigger (green networking terminal)
    bool deviceHasAlarmBus;       //!< Device has an alarm due to bus trigger (via KNX)
    bool deviceHasTestAlarmLocal; //!< Device has a test alarm due to button press or wired trigger
    bool deviceHasTestAlarmBus;   //!< Device has a test alarm due to bus trigger (via KNX)
    bool requestedAlarmBus;       //!< Desired alarm state per KNX bus
    bool requestedTestAlarmBus;   //!< Desired test alarm state per KNX bus
    bool ignoreBusAlarm;          //!< After Alarm Reset via KNX bus, ignore bus alarms for some time
    uint8_t alarmCounter;         //!< Countdown to next periodic sending of alarm in seconds
    uint8_t testAlarmCounter;     //!< Countdown to next periodic sending of test alarm in seconds
    uint8_t delayedAlarmCounter;  //!< Countdown to delayed alarm sending in seconds
};



#endif /* SMOKE_DETECTOR_ALARM_H_ */
