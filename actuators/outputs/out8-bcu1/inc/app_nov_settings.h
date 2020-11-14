/*
 *  app_nov_settings.h
 *
 *  handles non-volatile application specific settings which shall survive a power loss
 *
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef APP_NOV_SETTINGS_H_
#define APP_NOV_SETTINGS_H_

#include <sblib/eib.h>
#include "MemMapperMod.h"

typedef struct
{
    union
    {
        byte AppValues[4]; // to access settings in a loop // TODO make this dynamic, 4 is just stupid !!
        struct
        {
            unsigned char relaisstate;         // current relais state
            unsigned char handactuationstate;  // current hand actuation state
#ifdef DEBUG
            unsigned char testBusRestartCounter; //TODO rename or remove after testing
#endif
        };
    };
} AppNovSettingStruct;

class AppNovSetting
{
    public:
        AppNovSetting(unsigned int flashBase, unsigned int flashSize, unsigned int AppValueCacheSize);
        MemMapperMod* GetMemMapperMod();
        bool RecallAppData();
        bool StoreApplData();
    protected:

    private:
        AppNovSettingStruct AppSavedSettings; // holds all application specific settings which should survive a power loss
        MemMapperMod memMapper_;
        unsigned char crcSettings;
        unsigned char crc8();

};

#endif /* APP_NOV_SETTINGS_H_ */
