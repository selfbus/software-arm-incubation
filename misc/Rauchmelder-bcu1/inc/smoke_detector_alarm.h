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
    SmokeDetectorAlarm(SmokeDetectorConfig *config, SmokeDetectorGroupObjects *groupObjects);
    ~SmokeDetectorAlarm() = delete;

    bool hasAlarm() const;
    void groupObjectUpdated(GroupObject groupObject);
    void deviceStatusUpdate(bool newAlarmLocal, bool newTestAlarmLocal, bool newAlarmFromBus, bool newTestAlarmFromBus);
    void deviceButtonPressed();
    RmAlarmState process_alarm_stats();
    void timerEverySecond();
    void timerEveryMinute();

private:
    unsigned long read_obj_value(unsigned char objno);
    void send_obj_test_alarm(bool newAlarm);

private:
    SmokeDetectorConfig *config;
    SmokeDetectorGroupObjects *groupObjects;
    bool alarmLocal;                   //!< Flag für lokalen Alarm und Wired Alarm (über grüne Klemme / Rauchmelderbus)
    bool alarmBus;                     //!< Flag für remote Alarm über EIB
    bool testAlarmLocal;               //!< Flag für lokalen Testalarm und Wired Testalarm
    bool testAlarmBus;                 //!< Flag für remote Testalarm über EIB
    bool setAlarmBus;                  //!< Flag für den gewünschten Alarm Status wie wir ihn über den EIB empfangen haben
    bool setTestAlarmBus;              //!< Flag für den gewünschten Testalarm Status wie wir ihn über den EIB empfangen haben
    bool ignoreBusAlarm;               //!< Flag für Bus Alarm & -Testalarm ignorieren
    uint8_t alarmCounter;        //!< Countdown Zähler für zyklisches Senden eines Alarms.
    uint8_t TalarmCounter;       //!< Countdown Zähler für zyklisches Senden eines Testalarms.
    uint8_t delayedAlarmCounter; //!< Countdown Zähler für verzögertes Senden eines Alarms
};



#endif /* SMOKE_DETECTOR_ALARM_H_ */
