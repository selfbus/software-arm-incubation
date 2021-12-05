/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <OutputItem.h>
#include <sblib/digital_pin.h>

OutputItem::OutputItem(BcuBase* bcu, int port, int firstComIndex, PortConfig *config, GenericItem* nextItem) : GenericItem(bcu, port, firstComIndex, config, nextItem)
{
	pinMode(port, OUTPUT);
	digitalWrite(port, !config->OutInvert); // Später ggf. alten Zustand wiederherstellen aus Flash/EEProm

	bcu->comObjects->requestObjectRead(firstComIndex);
	if (config->OutBlink == PortOutBlinkObjAndOff || config->OutBlink == PortOutBlinkObjAndOn)
	{
		bcu->comObjects->requestObjectRead(firstComIndex + 1);
	}
};

void OutputItem::Loop(int updatedOjectNo)
{
	if (updatedOjectNo == firstComIndex || updatedOjectNo == firstComIndex + 1 || blinkActionTime < millis())
	{
		byte state = bcu->comObjects->objectRead(firstComIndex);
		if (config->OutInvert)
		{
			state = !state;
		}

		switch (config->OutBlink)
		{
		case PortOutBlinkNever:
			blinkActionTime = 0xFFFFFFFF;
			break;
		case PortOutBlinkIfOff:
			if (!state)
			{
				state = 2;
			}
			else
			{
				blinkActionTime = 0xFFFFFFFF;
			}
			break;
		case PortOutBlinkIfOn:
			if (state)
			{
				state = 2;
			}
			else
			{
				blinkActionTime = 0xFFFFFFFF;
			}
			break;
		case PortOutBlinkObjAndOff:
			if (!state && bcu->comObjects->objectRead(firstComIndex + 1))
			{
				state = 2;
			}
			else
			{
				blinkActionTime = 0xFFFFFFFF;
			}
			break;
		case PortOutBlinkObjAndOn:
			if (state && bcu->comObjects->objectRead(firstComIndex + 1))
			{
				state = 2;
			}
			else
			{
				blinkActionTime = 0xFFFFFFFF;
			}
			break;
		case PortOutBlinkAlways:
			state = 2;
			break;
		}

		switch (state)
		{
		case 0:
		case 1:
			digitalWrite(port, state);
			break;
		case 2:
			if (blinkActionTime == 0xFFFFFFFF || blinkActionTime < millis())
			{
				if (digitalRead(port))
				{
					digitalWrite(port, 0);
					blinkActionTime = millis() + config->OutBlinkOffTime;
				}
				else
				{
					digitalWrite(port, 1);
					blinkActionTime = millis() + config->OutBlinkOnTime;
				}
			}
			break;
		}
	}
}
