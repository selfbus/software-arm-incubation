/*
 * MemMapperMod.cpp - Modified MemMapper
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include <MemMapperMod.h>

int MemMapperMod::writeMemPtr(int virtAddress, byte *data, int length)
{
 if (virtAddress > 255)
  return MemMapper::writeMemPtr(virtAddress, data, length);
 else
  return MEM_MAPPER_INVALID_ADDRESS;
}

int MemMapperMod::readMemPtr(int virtAddress, byte *data, int length, bool forceFlash)
{
 if (virtAddress > 255)
  return MemMapper::readMemPtr(virtAddress, data, length, forceFlash);
 else
  return MEM_MAPPER_INVALID_ADDRESS;
}

bool MemMapperMod::isMapped(int virtAddress)
{
 if (virtAddress > 255)
  return MemMapper::isMapped(virtAddress);
 else
  return MEM_MAPPER_NOT_MAPPED;
}
