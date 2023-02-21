/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PORTCONFIG_H_
#define PORTCONFIG_H_

#include <DHTPinConfig.h>
#include <InputPinConfig.h>
#include <OutputPinConfig.h>
#include <PWMPinConfig.h>
#include <CounterPinConfig.h>

enum ARMPinType : byte
{
	ARMPinTypeInputFloating,
	ARMPinTypeInputPullup,
	ARMPinTypeInputPulldown,
	ARMPinTypeOutput,
	ARMPinTypeDHT11,
	ARMPinTypeDHT22,
	ARMPinTypePWM
};

#pragma pack(1)
struct ARMPinConfig
{
	struct
	{
		ARMPinType Type;
	} BaseConfig;

	union {
		DHTPinConfig DHT;
		InputPinConfig Input;
		OutputPinConfig Output;
		PWMPinConfig PWM;
		CounterPinConfig Counter;
	};
};
#pragma pack()

#endif /* PORTCONFIG_H_ */
