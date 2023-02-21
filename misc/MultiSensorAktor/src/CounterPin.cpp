/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <CounterPin.h>
#include <math.h>

CounterPin::CounterPin(BcuBase* bcu, byte firstComIndex, CounterPinConfig *config) : GenericPin(bcu, firstComIndex), config(config)
{
   	bcu->comObjects->requestObjectRead(firstComIndex);
}

void CounterPin::PutValue(uint32_t now, int val)
{
	int value = debouncer.value();
	if (value != lastValue && !value)
	{
		float val = bcu->comObjects->objectRead(firstComIndex);
		val += config->Increment;
		val = roundFloat(val, config->Decimals);
		bcu->comObjects->objectWrite(firstComIndex, (byte*)&val);

		if (lastTime < now)
		{
			bcu->comObjects->objectWrite(firstComIndex + 1, now - lastTime);
			float freq = 3600000.0f / (now - lastTime);
			freq = roundFloat(freq, config->DecimalsFreq);
			bcu->comObjects->objectWrite(firstComIndex + 2, (byte*)&freq);
		}
	}
}

float CounterPin::roundFloat(float val, int decimals)
{
	return floor(val * pow(10,decimals)) / pow(10,decimals);
}
