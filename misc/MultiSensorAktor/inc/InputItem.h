/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef INPUTITEM_H_
#define INPUTITEM_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/debounce.h>
#include <GenericItem.h>
#include <PortConfig.h>

class InputItem : public GenericItem
{
public:
	InputItem(BcuBase* bcu, int port, int firstComIndex, PortConfig* config, GenericItem* nextItem);
	~InputItem() = default;

	virtual void Loop(int updatedObjectNo);

protected:
	Debouncer debouncer;
	void setObj(int idx, byte item);
	unsigned int valueSet = 0;
	bool longActive;
	bool longPending;
};

#endif /* INPUTITEM_H_ */
