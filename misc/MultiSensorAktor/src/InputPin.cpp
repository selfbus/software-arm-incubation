/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>

#include <InputPin.h>
#include <HelperFunctions.h>

// Todos:
// Einstellung für Reset:
// - Aktuellen Wert melden
// - Ein
// - Aus
// - Wert abfragen
// Zyklisch senden



InputPin::InputPin(byte firstComObj, InputPinConfig *config, uint16_t& objRamPointer) : GenericPin(firstComObj), config(config)
{
	longActive = config->HiLong || config->LoLong;

	if (config->Hi == PortInToggle || config->Lo == PortInToggle)
	{
		BCU->comObjects->requestObjectRead(firstComObj);
	}
	BCU->comObjects->requestObjectRead(firstComObj + 2);
   	if (longActive && (config->HiLong == PortInToggle || config->LoLong == PortInToggle))
   	{
   		BCU->comObjects->requestObjectRead(firstComObj + 1);
   	}

   	debouncer.init(100);
   	debouncer.debounce(100, 0);

   	HelperFunctions::setComObjPtr(BCU, firstComIndex, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 1, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 2, BIT_1, objRamPointer);
}

void InputPin::PutValue(uint32_t now, int val)
{
	int lastValue = debouncer.value();
	int value = debouncer.debounce(val, config->DebounceTime);
	if (value != lastValue /*|| init*/)
	{
		// Short keypress with support for long press activated
		if (longActive && longPending && !config->ShortOnLong)
		{
			if (lastValue)
			{
				setObj(firstComIndex, config->Hi);
			}
			else
			{
				setObj(firstComIndex, config->Lo);
			}
		}

		valueSet = now;
		longPending = true;

		// short keypress with direct sending active or no long keypress supported
		if (config->ShortOnLong || !longActive)
		{
			if (value)
			{
				setObj(firstComIndex, config->Hi);
			}
			else
			{
				setObj(firstComIndex, config->Lo);
			}
		}
	}

    if (longActive && longPending && (valueSet + config->LongTime < now))
    {
    	longPending = false;
    	if (lastLongValue != value)
    	{
			if (value)
			{
				setObj(firstComIndex + 1, config->HiLong);
			}
			else
			{
				setObj(firstComIndex + 1, config->LoLong);
			}
			lastLongValue = value;
    	}
    }
}

void InputPin::setObj(int idx, byte item)
{
	switch (item)
	{
	case PortInNothing:
		break;
	case PortInOff:
		BCU->comObjects->objectWrite(idx, (unsigned int)0);
		break;
	case PortInOn:
		BCU->comObjects->objectWrite(idx, (unsigned int)1);
		break;
	case PortInToggle:
		BCU->comObjects->objectWrite(idx, !BCU->comObjects->objectRead(idx));
	}
}
