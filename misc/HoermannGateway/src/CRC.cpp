/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "CRC.h"

void CRC::reset()
{
	current = 0xF3;
}

void CRC::update(int c)
{
	uint8_t data = c ^ current;
	current = crcTable[data];
}

bool CRC::matches(int c)
{
	return current == c;
}
