/*
 * DHTPinConfig.h
 *
 *  Created on: 30.12.2021
 *      Author: dridders
 */

#ifndef DHTPINCONFIG_H_
#define DHTPINCONFIG_H_

#pragma pack(1)
struct DHTPinConfig
{
	unsigned short PreFan;
	unsigned short PreMeasure;
	unsigned short Delay;
	int16_t Offset;
};
#pragma pack()

#endif /* DHTPINCONFIG_H_ */
