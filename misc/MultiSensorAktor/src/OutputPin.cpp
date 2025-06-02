/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>

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
	setType(millis());

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

		setType(now);
	}
	else if (updatedObjectNo == firstComIndex + 1)
	{
		blinkObjState = BCU->comObjects->objectRead(firstComIndex + 1);

		setType(now);
	}
	else if (updatedObjectNo == firstComIndex + 2)
	{
		locked = BCU->comObjects->objectRead(firstComIndex + 2) == 0;
		if (config->lockFlags == PortOutLockAction::NegNothing ||
				config->lockFlags == PortOutLockAction::NegOff ||
				config->lockFlags == PortOutLockAction::NegOn ||
				config->lockFlags == PortOutLockAction::NegToggle)
		{
			locked = !locked;
		}

		if (locked)
		{
			switch (config->lockFlags)
			{
				case PortOutLockAction::PosNothing:
				case PortOutLockAction::NegNothing:
					break;
				case PortOutLockAction::PosOff:
				case PortOutLockAction::NegOff:
					sw = false;
					break;
				case PortOutLockAction::PosOn:
				case PortOutLockAction::NegOn:
					sw = true;
					break;
				case PortOutLockAction::PosToggle:
				case PortOutLockAction::NegToggle:
					sw = !sw;
					break;
			}
		}
	}

	if (blinkActionTime < now)
	{
		if (config->Blink == PortOutPulse)
		{
			if (sw && !locked)
			{
				blinkActionTime = 0xFFFFFFFF;
				blink = false;
				sw = false;
				BCU->comObjects->objectWrite(firstComIndex, sw);
			}
		}
		else
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
	}

	if (!locked)
	{
		if (blink)
		{
			lastState = blinkState;
		}
		else
		{
			lastState = sw;
		}
	}

	bool outVal = config->Invert ? !lastState : lastState;

	if (config->Blink != PortOutPulse && BCU->comObjects->objectRead(firstComIndex + 3) != sw)
	{
		BCU->comObjects->objectWrite(firstComIndex + 3, sw);
	}

	return outVal;
}

void OutputPin::setType(uint32_t now)
{
	switch (config->Blink)
	{
	case PortOutBlinkNever:
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
	case PortOutPulse:
		blink = false;
		if (sw)
		{
			blinkActionTime = millis() + config->BlinkOnTime;
		}
		return;
	}

	if (blink)
	{
		if (blinkActionTime == 0xFFFFFFFF)
		{
			blinkActionTime = now;
		}
	}
	else
	{
		blinkActionTime = 0xFFFFFFFF;
	}
}
