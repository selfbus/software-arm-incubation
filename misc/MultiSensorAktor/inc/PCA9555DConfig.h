/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PCA9555DCONFIG_H_
#define PCA9555DCONFIG_H_

#include <InputPin.h>
#include <OutputPin.h>
#include <CounterPin.h>
#include <PWMPin.h>

enum PCA9555DPinType : byte
{
	PCA9555DPinTypeInput,
	PCA9555DPinTypeOutput,
	PCA9555DPinTypePWM
};

#pragma pack(1)
struct PCA9555DPinConfig
{
	PCA9555DPinType Type;

	union {
		InputPinConfig Input;
		OutputPinConfig Output;
		PWMPinConfig PWM;
	};
};

struct PCA9555DConfig
{
	byte Address;
	byte IntPin;
	uint16_t PinEnabled;
};
#pragma pack()

#endif /* PCA9555DCONFIG_H_ */
