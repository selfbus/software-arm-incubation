/*
 *  tel_dump.cpp - Printing KNX packets as debug output
 *
 *  Copyright (C) 2021 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "hid_knx.h"
#include "tel_dump_usb.h"

TelDumpUsb teldump;

// Den evtl. langen String in 64 Byte Häppchen splitten und im Cdc-Fifo einreihen
void TelDumpUsb::OutputFunction(char* ptr, unsigned len) //Split_CdcEnqueue(char* ptr, unsigned len)
{
	Split_CdcEnqueue(ptr, len);
}





