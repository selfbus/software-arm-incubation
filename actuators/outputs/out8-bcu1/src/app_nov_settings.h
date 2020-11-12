/*
 *  app_nov_settings.h
 *
 *  handles non-volatile application specific settings which shall survive a power loss
 *
 *  Created on: 12.11.2020
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef APP_NOV_SETTINGS_H_
#define APP_NOV_SETTINGS_H_

#include <sblib/eib.h>

typedef struct
{
    union
    {
        byte AppValues[3]; // to access settings in a loop
        struct
        {
            byte relaisstate;         // current relais state
            byte handactuationstate;  // current hand actuation state
            byte crc;                 // TODO needs implementation of a simple crc check
#ifdef DEBUG
            byte testBusRestartCounter; //TODO remove after testing
#endif
        };
    };
} AppNovSetting;

extern AppNovSetting  AppSavedSettings; // holds all application specific settings which should survive a power loss

#endif /* APP_NOV_SETTINGS_H_ */
