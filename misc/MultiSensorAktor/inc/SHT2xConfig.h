/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SHT2XCONFIG_H_
#define SHT2XCONFIG_H_

#pragma pack(1)
struct SHT2xConfig
{
	uint16_t PreFan;
	uint16_t PreMeasure;
	uint16_t Delay;
	int16_t Offset;
};
#pragma pack()


#endif /* SHT2XCONFIG_H_ */
