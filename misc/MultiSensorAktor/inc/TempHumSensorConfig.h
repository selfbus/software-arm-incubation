/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef TEMPHUMSENSORCONFIG_H_
#define TEMPHUMSENSORCONFIG_H_

#pragma pack(1)
struct TempHumSensorConfig
{
	uint16_t PreFan;
	uint16_t PreMeasure;
	uint16_t Delay;
	int16_t Offset;
};
#pragma pack()


#endif /* TEMPHUMSENSORCONFIG_H_ */
