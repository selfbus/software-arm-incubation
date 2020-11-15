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


class NonVolatileSetting
{
    public:
        NonVolatileSetting(unsigned int flashBase, unsigned int flashSize, unsigned int AppValueCacheSize);
        MemMapperMod* GetMemMapperMod();
        bool RecallAppData(unsigned char *appdata, unsigned int size);
        bool StoreApplData(unsigned char *appdata, unsigned int size);
    protected:

    private:
        MemMapperMod memMapper_;
        NonVolatileSetting(){};
        unsigned char crc8(unsigned char *data, unsigned int size);

};

#endif /* APP_NOV_SETTINGS_H_ */
