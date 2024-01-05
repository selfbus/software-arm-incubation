/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef GENERICITEM_H_
#define GENERICITEM_H_

#include <sblib/eib/bcu_base.h>

class GenericItem
{
public:
	GenericItem(byte firstComIndex, GenericItem* nextItem) : nextItem(nextItem), firstComIndex(firstComIndex) {}

	virtual void Loop(uint32_t now, int updatedObjNo) = 0;
//	virtual void Irq(void* item, byte newValue) {}
	GenericItem* GetNextItem() { return nextItem; };

	virtual int ConfigLength() = 0;
	virtual int ComObjCount() = 0;

	static BcuBase* BCU;
protected:
	GenericItem* nextItem;
//	BcuBase* bcu;
	byte firstComIndex;
};

#endif /* GENERICITEM_H_ */
