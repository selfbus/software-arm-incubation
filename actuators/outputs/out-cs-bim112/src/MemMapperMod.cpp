/*
 * MemMapperMod.cpp - Modified MemMapper
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include <MemMapperMod.h>

int MemMapperMod::writeMemPtr(int virtAddress, byte *data, int length)
{
 if ((virtAddress >= 0x4B20) && (virtAddress < 0x8000))
 { // Hier wird ein größerer Speicherbereich ab 0x4B20 emuliert.
  // Die Daten werden Modulo-mäßig in einen kleinen 32 Byte Bereich geschrieben.
  // Das nutzt die Eigenschaft der ETS aus, immer kleine Bereiche zu schreiben
  // und sofort zu verifizieren.
  for (int i=virtAddress; i < (virtAddress+length); i++)
  {
   fakemem[i & 31] = *data++;
  }
  return MEM_MAPPER_SUCCESS;
 } else
 if (virtAddress > 255)
  return MemMapper::writeMemPtr(virtAddress, data, length);
 else
  return MEM_MAPPER_INVALID_ADDRESS;
}

int MemMapperMod::readMemPtr(int virtAddress, byte *data, int length, bool forceFlash)
{
 if ((virtAddress >= 0x4B20) && (virtAddress < 0x8000))
 { // Hier wird ein größerer Speicherbereich ab 0x4B20 emuliert.
  // Die Daten werden Modulo-mäßig in einen kleinen 32 Byte Bereich geschrieben.
  // Das nutzt die Eigenschaft der ETS aus, immer kleine Bereiche zu schreiben
  // und sofort zu verifizieren.
  for (int i=virtAddress; i < (virtAddress+length); i++)
  {
   *data++ = fakemem[i & 31];
  }
  return MEM_MAPPER_SUCCESS;
 } else
  if (virtAddress > 255)
   return MemMapper::readMemPtr(virtAddress, data, length, forceFlash);
  else
   return MEM_MAPPER_INVALID_ADDRESS;
}

bool MemMapperMod::isMapped(int virtAddress)
{
 // Hier wird ein größerer Speicherbereich ab 0x4B20 emuliert.
 if ((virtAddress >= 0x4B20) && (virtAddress < 0x8000))
  return true;
 else
  if (virtAddress > 255)
   return MemMapper::isMapped(virtAddress);
  else
   return false;
}
