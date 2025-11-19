/*
 *  tel_dump.h - Printing KNX packets as debug output
 *
 *  Copyright (C) 2021-2021 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef TEL_DUMP_USB_H_
#define TEL_DUMP_USB_H_

#include "tel_dump.h"

class TelDumpUsb: public TelDump
{
protected:
    virtual void OutputFunction(char *line, unsigned int len);
};

extern TelDumpUsb teldump;


#endif /* TEL_DUMP_USB_H_ */
