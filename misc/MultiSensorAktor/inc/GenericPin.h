/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef GENERICPIN_H_
#define GENERICPIN_H_

#include <sblib/eib/bcu_base.h>

class GenericPin
{
public:
	GenericPin(byte firstComIndex) : firstComIndex(firstComIndex) {};
	~GenericPin() = default;

	virtual byte GetState(uint32_t now, byte updatedObjectNo) { return 0; };
	virtual void PutValue(uint32_t now, int val) {};

	virtual int ConfigLength() = 0;
	virtual int ComObjCount() = 0;

	static BcuBase* BCU;

protected:
	byte firstComIndex;
};

#endif /* GENERICPIN_H_ */
