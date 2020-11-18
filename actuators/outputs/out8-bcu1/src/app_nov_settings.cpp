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

NonVolatileSetting::NonVolatileSetting(unsigned int flashBase, unsigned int flashSize, unsigned int CacheSize)
 : memMapper_(flashBase, flashSize)
{
    memMapper_.addRange(0x0, CacheSize);
}

MemMapperMod* NonVolatileSetting::GetMemMapperMod()
{
    return &memMapper_;
}

bool NonVolatileSetting::RecallAppData(unsigned char *appdata, unsigned int size)
{
    /* TODO from Slack by Mirko
     * Wenn man sich doppelt soviel Blöcke gönnen kann
     * könnte man die Backup "Bank" immer schon zu Programmstart löschen.
     * Beim Backup dann eine Sequenznumner + Checksumme schreiben.
     * Die größte Sequenznummer ist dann die gültige Konfiguration.
     */

    byte *StoragePtr;
    StoragePtr = memMapper_.memoryPtr(0, false);

    unsigned char* p_save = appdata; // save address of appdata
    for (unsigned int i = 0; i < size; i++)
    {
        *appdata++ = *StoragePtr++;
    }
    appdata = p_save; // restore address of appdata

    // restore crc from last byte
    unsigned char crc = *StoragePtr;

    return (crc == crc8(appdata, size));
}

bool NonVolatileSetting::StoreApplData(unsigned char *appdata, unsigned int size)
{
    byte* StoragePtr;
    // Kann der Mapper überhaupt die Seite 0 mappen? Checken!
    memMapper_.writeMem(0, 0); // writeMem() aktiviert die passende Speicherseite, entgegen zu memoryPtr()
    StoragePtr = memMapper_.memoryPtr(0, false);


    unsigned char* p_save = appdata; // save address of appdata
    unsigned char crc = crc8(appdata, size); // calculate simple crc
    for (unsigned int i = 0; i < size; i++)
    {
        *StoragePtr++ = *appdata++;
    }
    appdata = p_save; // restore address of appdata
    // save crc as last byte
    *StoragePtr++ = crc;

    return (memMapper_.doFlash() > 0); // Erase time for one sector is 100 ms ± 5%. Programming time for one block of 256
                                       // bytes is 1 ms ± 5%. see manual page 407
}

unsigned char NonVolatileSetting::crc8(unsigned char *data, unsigned int size)
{
   unsigned char crc = 0;
   unsigned char extract;
   unsigned char sum;
   for(int i=0; i<size; i++)
   {
      extract = *data++;
      for (char tempI = 8; tempI; tempI--)
      {
         sum = (crc ^ extract) & 0x01;
         crc >>= 1;
         if (sum)
             crc ^= 0x8C;
         extract >>= 1;
      }
   }
   return crc;
}
