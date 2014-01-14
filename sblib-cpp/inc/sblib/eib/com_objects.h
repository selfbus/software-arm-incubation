/*
 *  com_objects.h - EIB Communication objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_com_objects_h
#define sblib_com_objects_h

#include <sblib/eib/bcu.h>
#include <sblib/eib/types.h>

// Alternative API:

int objectRead(int objno);
void objectWrite(int objno, int value);

float objectReadFloat(int objno);
void objectWrite(int objno, float value);

byte* objectReadArray(int objno);
void objectWrite(int objno, byte* value);

ComType objectType(int objno);
int objectSize(int objno);

/**
 * Get the index of the next communication object that was
 * updated via the bus.
 *
 * @return The index of the next updated com-object, -1 if none.
 */
int nextUpdatedObject();

/**
 * Process a multicast group telegram.
 *
 * This function is called by bcu.processTelegram(). It is usually not required to call
 * this function from within a user program. The telegram that is processed is read from
 * bus.telegram[].
 *
 * @param addr - the destination group address.
 */
void processGroupTelegram(int addr, int apci);

/**
 * Get the communication object flags table. This is the table with the
 * flags that are configured by ETS (not the RAM status flags).
 *
 * @return The com-objects flags table.
 */
byte* objectFlagsTable();

#endif /*sblib_com_objects_h*/
