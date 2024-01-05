/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <OutputPin.h>
#include <HelperFunctions.h>

OutputPin::OutputPin(byte firstComIndex, OutputPinConfig* config, uint16_t& objRamPointer) : GenericPin(firstComIndex), config(config), sw(false), blinkObjState(false), blinkState(false), blink(false), lastState(false), blinkActionTime(0)
{
	BCU->comObjects->requestObjectRead(firstComIndex);
	BCU->comObjects->requestObjectRead(firstComIndex + 2);
	if (config->Blink == PortOutBlinkObjAndOff || config->Blink == PortOutBlinkObjAndOn)
	{
		BCU->comObjects->requestObjectRead(firstComIndex + 1);
	}
	setType();

	HelperFunctions::setComObjPtr(BCU, firstComIndex, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 1, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 2, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 3, BIT_1, objRamPointer);
}

byte OutputPin::GetState(uint32_t now, byte updatedObjectNo)
{
	if (updatedObjectNo == firstComIndex)
	{
		sw = BCU->comObjects->objectRead(firstComIndex) != 0;

		setType();
	}
	else if (updatedObjectNo == firstComIndex + 1)
	{
		blinkObjState = BCU->comObjects->objectRead(firstComIndex + 1);

		setType();
	}

	if (blinkActionTime < now)
	{
		blinkState = !blinkState;
		if (blinkState)
		{
			blinkActionTime += config->BlinkOnTime;
		}
		else
		{
			blinkActionTime += config->BlinkOffTime;
		}
	}

	if (blink)
	{
		lastState = blinkState;
	}
	else
	{
		lastState = sw;
	}

	return config->Invert ? !lastState : lastState;
}

void OutputPin::setType()
{
	switch (config->Blink)
	{
	case PortOutBlinkNever:
		blinkActionTime = 0xFFFFFFFF;
		blink = false;
		break;
	case PortOutBlinkIfOff:
		blink = (!sw);
		break;
	case PortOutBlinkIfOn:
		blink = (sw);
		break;
	case PortOutBlinkObj:
		blink = blinkObjState;
		break;
	case PortOutBlinkObjAndOff:
		blink =  (!sw && blinkObjState);
		break;
	case PortOutBlinkObjAndOn:
		blink =  (sw && blinkObjState);
		break;
	case PortOutBlinkAlways:
		blink = true;
		break;
	}
}
