/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef COUNTERPINCONFIG_H_
#define COUNTERPINCONFIG_H_

#pragma pack(1)

struct CounterPinConfig
{
	byte Decimals;
	byte DecimalsFreq;
	byte Increment;
};
#pragma pack()

#endif /* COUNTERPINCONFIG_H_ */
