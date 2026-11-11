/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>

#include <PWMPin.h>
#include <PwmController.h>
#include <HelperFunctions.h>

PWMPin::PWMPin(uint8_t firstComIndex, PWMPinConfig* config, GenericItem* parent, uint16_t& objRamPointer,
               int pwmChannel) : GenericPin(firstComIndex), config(config), parent(parent), pwmChannel(pwmChannel)
{
	comObjects->requestObjectRead(firstComIndex);
	comObjects->requestObjectRead(firstComIndex + 1);

	HelperFunctions::setComObjPtr(comObjects, firstComIndex, BYTE_1, objRamPointer);
	HelperFunctions::setComObjPtr(comObjects, firstComIndex + 1, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(comObjects, firstComIndex + 2, BIT_1, objRamPointer);
}

uint8_t PWMPin::GetState(uint32_t now, uint8_t updatedObjectNo)
{
	if (updatedObjectNo > 0 && (updatedObjectNo == firstComIndex || updatedObjectNo == firstComIndex + 1))
	{
		value = targetValue(*config, comObjects->objectRead(firstComIndex), comObjects->objectRead(firstComIndex + 1),
		                    value);
		pwmController.setValue(pwmChannel, value); // ignores -1
	}
	return 0;
}

uint8_t PWMPin::targetValue(const PWMPinConfig& config, const uint8_t objectValue, const bool lock,
                            const uint8_t current)
{
	switch (config.FixedValueSwitch)
	{
	case PortFixedPwmValueNever:
		return objectValue;
	case PortFixedPwmValueOnLock:
		return lock ? objectValue : config.FixedValueOnOn;
	case PortFixedPwmValueAlways:
		return objectValue ? config.FixedValueOnOff : config.FixedValueOnOn;
	}
	return current;
}

bool PWMPin::inverted(const PWMPinConfig& config)
{
	return reinterpret_cast<const uint8_t&>(config.Invert) != 0;
}
