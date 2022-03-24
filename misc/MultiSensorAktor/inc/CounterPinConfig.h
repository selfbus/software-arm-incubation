/*
 * CounterPinConfig.h
 *
 *  Created on: 30.12.2021
 *      Author: dridders
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
