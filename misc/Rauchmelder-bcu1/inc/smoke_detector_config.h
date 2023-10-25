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

    bool alarmSendStatusPeriodically() const;
    bool alarmSendStatusPeriodicallyWhenNoAlarm() const;
    bool alarmSendNetworkPeriodically() const;
    uint8_t alarmIntervalSeconds() const;
    bool alarmSendDelayed() const;
    uint8_t alarmDelaySeconds() const;

    bool testAlarmSendStatusPeriodically() const;
    bool testAlarmSendNetworkPeriodically() const;
    uint8_t testAlarmIntervalSeconds() const;

    bool infoSendAnyPeriodically() const;
    uint8_t infoIntervalMinutes() const;
    bool infoSendPeriodically(GroupObject groupObject) const;
    bool infoSendOperationTimeInHours() const;

    int8_t temperatureOffsetInTenthDegrees() const;

private:
    Memory *memory;
};

#endif /* smoke_detector_config_h */
