/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef COUNTERPIN_H_
#define COUNTERPIN_H_

#include <sblib/eib/bcu_base.h>
#include <CounterPinConfig.h>
#include <GenericPin.h>

class CounterPin : public GenericPin
{
public:
	CounterPin(byte firstComIndex, CounterPinConfig *config);
	~CounterPin() = default;

	void PutValue(uint32_t now, int val);

	int ConfigLength() { return sizeof(CounterPinConfig); }
	int ComObjCount() { return 3; }

protected:
	CounterPinConfig* config;

	bool lastValue = false;
	uint32_t lastTime = 0xFFFFFFFF;

	float roundFloat(float val, int decimals);

	Debouncer debouncer = Debouncer();
};

#endif /* COUNTERPIN_H_ */
