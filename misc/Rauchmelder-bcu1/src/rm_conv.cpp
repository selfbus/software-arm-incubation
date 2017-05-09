/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include "rm_conv.h"

/**
 * Einen Wert auf DPT 9.001 2 Byte Float Format wandeln
 */
unsigned long conv_dpt_9_001(unsigned long val)
{
	unsigned int div = 0;
	while (val >= 2047)
	{
		++div;
		val >>= 1;
	}
	val |= div << 11;
	return val;
}
