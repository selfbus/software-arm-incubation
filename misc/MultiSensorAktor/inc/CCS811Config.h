/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CCS811CONFIG_H_
#define CCS811CONFIG_H_

#pragma pack(1)
struct CCS811Config
{
	uint16_t PreFan;
	uint16_t PreMeasure;
	uint16_t Delay;
	byte addr;
	byte wakePin;
};
#pragma pack()


#endif /* CCS811CONFIG_H_ */
