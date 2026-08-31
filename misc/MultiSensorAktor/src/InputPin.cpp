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

InputPin::InputPin(byte firstComObj, InputPinConfig *config, uint16_t& objRamPointer,
	InputPinForcedConfig *forcedConfig) :
	GenericPin(firstComObj), config(config), forcedConfig(forcedConfig)
{
	// DPT 2.001 and the dedicated toggle feedback objects are extensions of
	// the native ARM input only. PCA9555D keeps its legacy configuration and
	// four communication objects because forcedConfig is null there.
	if (forcedConfig != nullptr)
	{
		shortActionIs2Bit = (config->LockFlags & PortInShortAction2Bit) != 0;
		longActionIs2Bit = (config->LockFlags & PortInLongAction2Bit) != 0;
	}

	longActive = longAction(true) != PortInNothing || longAction(false) != PortInNothing;
	shortToggleActive = forcedConfig != nullptr && !shortActionIs2Bit &&
		(config->Hi == PortInToggle || config->Lo == PortInToggle);
	longToggleActive = forcedConfig != nullptr && !longActionIs2Bit && longActive &&
		(config->HiLong == PortInToggle || config->LoLong == PortInToggle);

	debouncer.init(100);
	debouncer.debounce(100, 0);

	HelperFunctions::setComObjPtr(BCU, firstComIndex + ShortObject,
		shortActionIs2Bit ? BIT_2 : BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + LongObject,
		longActionIs2Bit ? BIT_2 : BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + LockObject, BIT_1, objRamPointer);

	if (shortToggleActive)
	{
		HelperFunctions::setComObjPtr(BCU, firstComIndex + ShortStatusObject, BIT_1, objRamPointer);
		BCU->comObjects->requestObjectRead(firstComIndex + ShortStatusObject);
	}
	if (longToggleActive)
	{
		HelperFunctions::setComObjPtr(BCU, firstComIndex + LongStatusObject, BIT_1, objRamPointer);
		BCU->comObjects->requestObjectRead(firstComIndex + LongStatusObject);
	}

	BCU->comObjects->requestObjectRead(firstComIndex + LockObject);
}

byte InputPin::shortAction(bool high) const
{
	if (shortActionIs2Bit && forcedConfig != nullptr)
	{
		return high ? forcedConfig->Hi : forcedConfig->Lo;
	}
	return high ? config->Hi : config->Lo;
}

byte InputPin::longAction(bool high) const
{
	if (longActionIs2Bit && forcedConfig != nullptr)
	{
		return high ? forcedConfig->HiLong : forcedConfig->LoLong;
	}
	return high ? config->HiLong : config->LoLong;
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
			setObj(firstComIndex + ShortObject, shortAction(lastValue != 0), shortActionIs2Bit,
				shortToggleActive ? firstComIndex + ShortStatusObject : -1);
		}

		valueSet = now;
		longPending = true;
		// short keypress with direct sending active or no long keypress supported
		if (config->ShortOnLong || !longActive)
		{
			setObj(firstComIndex + ShortObject, shortAction(value != 0), shortActionIs2Bit,
				shortToggleActive ? firstComIndex + ShortStatusObject : -1);
		}
	}

	if (longActive && longPending && (valueSet + config->LongTime < now))
	{
		longPending = false;
		if (lastLongValue != value)
		{
			setObj(firstComIndex + LongObject, longAction(value != 0), longActionIs2Bit,
				longToggleActive ? firstComIndex + LongStatusObject : -1);
			lastLongValue = value;
		}
	}
}

void InputPin::setObj(int idx, byte item, bool is2Bit, int statusIdx)
{
	if (is2Bit)
	{
		if (item >= PortInForcedNoPriorityOff && item <= PortInForcedPriorityOn)
		{
			// ETS stores 1..4 while DPT 2.001 transports 0..3.
			BCU->comObjects->objectWrite(idx, static_cast<unsigned int>(item - 1));
		}
		return;
	}

	unsigned int value = 0;
	bool send = true;

	switch (item)
	{
	case PortInNothing:
		send = false;
		break;
	case PortInOff:
		value = 0;
		break;
	case PortInOn:
		value = 1;
		break;
	case PortInToggle:
		// Native ARM inputs use a dedicated feedback object. Other users of
		// InputPin (currently PCA9555D) retain the original local-object
		// toggle behaviour.
		value = !BCU->comObjects->objectRead(statusIdx >= 0 ? statusIdx : idx);
		break;
	default:
		send = false;
		break;
	}

	if (!send)
	{
		return;
	}

	BCU->comObjects->objectWrite(idx, value);

	// Optimistically update the local feedback cache so two quick toggles work
	// even before the actuator reports its state. A real status telegram can
	// overwrite this value at any time.
	if (statusIdx >= 0)
	{
		BCU->comObjects->objectSetValue(statusIdx, value);
	}
}
