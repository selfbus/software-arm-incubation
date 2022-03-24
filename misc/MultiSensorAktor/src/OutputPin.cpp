/*
 * OutputPin.cpp
 *
 *  Created on: 27.12.2021
 *      Author: dridders
 */

#include <OutputPin.h>

OutputPin::OutputPin(BcuBase* bcu, byte firstComIndex, OutputPinConfig* config) : GenericPin(bcu, firstComIndex), config(config), sw(false), blinkObjState(false), blinkState(false), blink(false), lastState(false), blinkActionTime(0)
{
	bcu->comObjects->requestObjectRead(firstComIndex);
	bcu->comObjects->requestObjectRead(firstComIndex + 2);
	if (config->Blink == PortOutBlinkObjAndOff || config->Blink == PortOutBlinkObjAndOn)
	{
		bcu->comObjects->requestObjectRead(firstComIndex + 1);
	}
	setType();
}

byte OutputPin::GetState(uint32_t now, byte updatedObjectNo)
{
	if (updatedObjectNo == firstComIndex)
	{
		sw = bcu->comObjects->objectRead(firstComIndex) != 0;

		setType();
	}
	else if (updatedObjectNo == firstComIndex)
	{
		blinkObjState = bcu->comObjects->objectRead(firstComIndex + 1);

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
