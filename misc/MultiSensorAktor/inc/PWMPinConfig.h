/*
 * PWMPinConfig.h
 *
 *  Created on: 30.12.2021
 *      Author: dridders
 */

#ifndef PWMPINCONFIG_H_
#define PWMPINCONFIG_H_

enum PortFixedPwmValue : byte
{
	PortFixedPwmValueNever,
	PortFixedPwmValueOnLock = 128,
	PortFixedPwmValueAlways
};

#pragma pack(1)
struct PWMPinConfig
{
	bool Invert;
	PortFixedPwmValue FixedValueSwitch;
	byte FixedValueOnOn;
	byte FixedValueOnOff;
};
#pragma pack()

#endif /* PWMPINCONFIG_H_ */
