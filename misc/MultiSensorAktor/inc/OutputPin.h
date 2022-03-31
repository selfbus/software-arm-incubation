/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef OUTPUTPIN_H_
#define OUTPUTPIN_H_

#include <sblib/eib/bcu_base.h>
#include <OutputPinConfig.h>
#include <GenericPin.h>

class OutputPin : public GenericPin
{
public:
	OutputPin(BcuBase* bcu, byte firstComObj, OutputPinConfig *config);
	~OutputPin() = default;

	byte GetState(uint32_t now, byte updatedObjectNo);

	int ConfigLength() { return sizeof(OutputPinConfig); }
	int ComObjCount() { return 3; }

protected:
	OutputPinConfig* config;

	uint32_t blinkActionTime = 0;
	bool lastState = false;
	bool blink = false;
	bool blinkState = false;
	bool blinkObjState = false;
	bool sw = false;

	void setType();
};

#endif /* OUTPUTPIN_H_ */
