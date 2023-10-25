/*
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef smoke_detector_config_h
#define smoke_detector_config_h

#include "rm_const.h"
#include <sblib/eib/memory.h>

class SmokeDetectorConfig
{
public:
    SmokeDetectorConfig(Memory *memory);
    ~SmokeDetectorConfig() = delete;

    bool alarmSendStatusPeriodically();
    bool alarmSendStatusPeriodicallyWhenNoAlarm();
    bool alarmSendNetworkPeriodically();
    uint8_t alarmIntervalSeconds();
    bool alarmSendDelayed();
    uint8_t alarmDelaySeconds();

    bool testAlarmSendStatusPeriodically();
    bool testAlarmSendNetworkPeriodically();
    uint8_t testAlarmIntervalSeconds();

    bool infoSendAnyPeriodically();
    uint8_t infoIntervalMinutes();
    bool infoSendPeriodically(GroupObject groupObject);
    bool infoSendOperationTimeInHours();

    int8_t temperatureOffsetInTenthDegrees();

private:
    Memory *memory;
};

#endif /* smoke_detector_config_h */
