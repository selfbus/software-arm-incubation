/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <InputItem.h>
#include <sblib/digital_pin.h>

InputItem::InputItem(BcuBase* bcu, int port, int firstComIndex, PortConfig *config, GenericItem* nextItem) : GenericItem(bcu, port, firstComIndex, config, nextItem)
{
	switch (config->Type)
	{
	case PortTypeInputFloating:
		pinMode(port, INPUT);
		break;
	case PortTypeInputPullup:
		pinMode(port, INPUT | PULL_UP);
		break;
	case PortTypeInputPulldown:
		pinMode(port, INPUT | PULL_DOWN);
		break;
	}

	debouncer.init(digitalRead(port));

   	bcu->comObjects->requestObjectRead(firstComIndex);
   	bcu->comObjects->requestObjectRead(firstComIndex+1);

   	longActive = config->InLoLong != PortInNothing || config->InHiLong != PortInNothing;
 };

void InputItem::Loop(int updatedOjectNo)
{
    int lastValue = debouncer.value();
    int value = debouncer.debounce(digitalRead(port), config->DebounceTime);

    if (lastValue != value)
    {
    	// Short keypress with support for long press activated
    	if (valueSet + config->InLongTime > millis() && !longActive && !config->InShortOnLong)
    	{
    		if (lastValue)
    		{
    			setObj(firstComIndex, config->InHi);
    		}
    		else
    		{
    			setObj(firstComIndex, config->InLo);
    		}
    	}

		valueSet = millis();
    	longPending=true;

    	// short keypress with direct sending active or no long keypress supported
    	if (value && (config->InShortOnLong || !longActive))
		{
			setObj(firstComIndex, config->InHi);
		}
    	else if (!value && (config->InShortOnLong || !longActive))
    	{
			setObj(firstComIndex, config->InLo);
    	}
    }

    if (longActive && longPending && valueSet + config->InLongTime < millis())
    {
    	longPending=false;
    	if (value)
		{
			setObj(firstComIndex + 1, config->InHiLong);
		}
    	else if (!value)
    	{
			setObj(firstComIndex + 1, config->InLoLong);
    	}
    }
}

void InputItem::setObj(int idx, byte item)
{
	switch (item)
	{
	case PortInNothing:
		break;
	case PortInOff:
		bcu->comObjects->objectWrite(idx, (unsigned int)0);
		break;
	case PortInOn:
		bcu->comObjects->objectWrite(idx, (unsigned int)1);
		break;
	case PortInToggle:
		bcu->comObjects->objectWrite(idx, !bcu->comObjects->objectRead(idx));
	}
}
