/*
 *    _____ ________    __________  __  _______    ____  __  ___
 *   / ___// ____/ /   / ____/ __ )/ / / / ___/   / __ \/  |/  /
 *   \__ \/ __/ / /   / /_  / __  / / / /\__ \   / /_/ / /|_/ /
 *  ___/ / /___/ /___/ __/ / /_/ / /_/ /___/ /  / _, _/ /  / /
 * /____/_____/_____/_/   /_____/\____//____/  /_/ |_/_/  /_/
 *
 *  Original written for LPC922:
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

#ifndef SMOKE_DETECTOR_APP_H_
#define SMOKE_DETECTOR_APP_H_

#include <sblib/eibBCU1.h>

#include "rm_const.h"

class SmokeDetectorConfig;
class SmokeDetectorGroupObjects;
class SmokeDetectorAlarm;
class SmokeDetectorErrorCode;
class SmokeDetectorDevice;

class SmokeDetectorApp
{
public:
    SmokeDetectorApp();

    BcuBase* initialize();
    void loop();
    void loopNoApp();
    void timer();

private:
    void setupPeriodicTimer(uint32_t milliseconds);
    void updateAlarmState();
    void handleUpdatedGroupObjects();

private:
    BCU1 bcu;
    SmokeDetectorConfig *config;
    SmokeDetectorGroupObjects *groupObjects;
    SmokeDetectorAlarm *alarm;
    SmokeDetectorErrorCode *errorCode; //!< Smoke detector error code handling
    SmokeDetectorDevice *device;
    uint8_t infoCounter;               //!< Countdown to next periodic sending of informational group objects
    GroupObject infoSendObjno;         //!< Group object to be checked/sent next on periodic sending
    uint8_t readCmdno;                 //!< Number of the command to send to the smoke detector next
    uint8_t eventTime;                 //!< Half-second counter 0..119
};

#endif /*SMOKE_DETECTOR_APP_H_*/
