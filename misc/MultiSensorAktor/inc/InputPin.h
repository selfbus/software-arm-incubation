/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef INPUTPIN_H_
#define INPUTPIN_H_

#include <sblib/eib/bcu_base.h>
#include <InputPinConfig.h>
#include <GenericPin.h>

class InputPin : public GenericPin
{
public:
	InputPin(BcuBase* bcu, byte firstComObj, InputPinConfig *config, uint16_t& objRamPointer);
	~InputPin() = default;

	void PutValue(uint32_t now, int val);

	int ConfigLength() { return sizeof(InputPinConfig); }
	int ComObjCount() { return 3; }

protected:
	void setObj(int idx, byte item);

	InputPinConfig* config;

	Debouncer debouncer = Debouncer();
	bool longActive = false;
	bool longPending = false;
	uint32_t valueSet = 0;
	byte lastLongValue = 0;
};

#endif /* INPUTPIN_H_ */
