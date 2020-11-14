/*
 *  app_nov_settings.cpp
 *
 *  handles non-volatile application specific settings which shall survive a power loss
 *
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_nov_settings.h"

AppNovSetting::AppNovSetting(unsigned int flashBase, unsigned int flashSize, unsigned int AppValueCacheSize)
 : memMapper_(flashBase, flashSize)
{
    memMapper_.addRange(0x0, AppValueCacheSize);
}

MemMapperMod* AppNovSetting::GetMemMapperMod()
{
    return &memMapper_;
}

bool AppNovSetting::RecallAppData()
{
    /* TODO from Slack by Mirko
     * Wenn man sich doppelt soviel Blöcke gönnen kann
     * könnte man die Backup "Bank" immer schon zu Programmstart löschen.
     * Beim Backup dann eine Sequenznumner + Checksumme schreiben.
     * Die größte Sequenznummer ist dann die gültige Konfiguration.
     */

    byte* StoragePtr;
    StoragePtr = memMapper_.memoryPtr(0, false);
    for (unsigned int i = 0; i < sizeof(AppSavedSettings); i++)
    {
        AppSavedSettings.AppValues[i] = *StoragePtr++;
    }

    // restore crc from last byte
    unsigned char crc = *StoragePtr++;

    return (crc == crc8());
}

bool AppNovSetting::StoreApplData()
{
    byte* StoragePtr;
    // Kann der Mapper überhaupt die Seite 0 mappen? Checken!
    memMapper_.writeMem(0, 0); // writeMem() aktiviert die passende Speicherseite, entgegen zu memoryPtr()
    StoragePtr = memMapper_.memoryPtr(0, false);

#ifdef DEBUG
    AppSavedSettings.testBusRestartCounter++; // TODO remove after testing
#endif

    // calculate simple crc
    unsigned char crc = crc8();

    for (unsigned int i = 0; i < sizeof(AppSavedSettings); i++)
    {
        *StoragePtr++ = AppSavedSettings.AppValues[i];
    }
    // save crc as last byte
    *StoragePtr++ = crc;

    memMapper_.doFlash(); // Erase time for one sector is 100 ms ± 5%. Programming time for one block of 256
                          // bytes is 1 ms ± 5%. see manual page 407

    return true;
}

unsigned char AppNovSetting::crc8()
{
   crcSettings = 0;
   unsigned char extract;
   unsigned char sum;
   for(int i=0; i<sizeof(AppSavedSettings.AppValues); i++)
   {
      extract = AppSavedSettings.AppValues[i];
      for (char tempI = 8; tempI; tempI--)
      {
         sum = (crcSettings ^ extract) & 0x01;
         crcSettings >>= 1;
         if (sum)
             crcSettings ^= 0x8C;
         extract >>= 1;
      }
   }
   return crcSettings;
}
