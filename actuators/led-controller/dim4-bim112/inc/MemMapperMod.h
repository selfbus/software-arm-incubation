/*
 * MemMapperMod.h - Modified MemMapper
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 *  Modifies the functionality of the MemMapper-Class
 *  - Creates a memory page which is unaccessible from the bus to store local data
 *    (This is done by additional address-filtering for isMapped(), writeMemPtr(), readMemPtr().
 *    These three functions are used inside bcu.cpp)
 *    Page 0 (addr 0..255) is used for the page with access restrictions
 *  - Creates a dummy memory from 0x4B20 upwards
 */

#ifndef MEMMAPPERMOD_H_
#define MEMMAPPERMOD_H_

#include <sblib/mem_mapper.h>

class MemMapperMod: public MemMapper {
public:
 MemMapperMod(unsigned int flashBase = 0xe000, unsigned int flashSize = 0x1000) : MemMapper(flashBase, flashSize, false) {};

 /*
  * If virtAddress > 255, MemMapper::writeMemPtr() is called. This function forbids access
  * to virtAdress <= 255 to implement a local storage which can not be accessed from the bus.
  * To access the local storage, other functions has to be used.
  */
 virtual int writeMemPtr(int virtAddress, byte *data, int length);

 /*
  * If virtAddress > 255, MemMapper::readMemPtr() is called. This function forbids access
  * to virtAdress <= 255 to implement a local storage which can not be accessed from the bus.
  * To access the local storage, other functions has to be used.
  */
virtual int readMemPtr(int virtAddress, byte *data, int length, bool forceFlash =
         false);

/*
 * If virtAddress > 255, MemMapper::isMapped() is called. This function forbids access
 * to virtAdress <= 255 to implement a local storage which can not be accessed from the bus.
  * To access the local storage, other functions has to be used.
 */
virtual bool isMapped(int virtAddress);

protected:
 byte fakemem[32];
};

#endif /* MEMMAPPERMOD_H_ */
