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
#include <ARMPinConfig.h>
#include <GenericPin.h>

class ARMPinItem : public GenericItem
{
public:
	ARMPinItem(BcuBase* bcu, int port, byte firstComIndex, ARMPinConfig* config, GenericItem* nextItem);
	~ARMPinItem() = default;

	void Loop(uint32_t now, int updatedObjectNo);
	void Irq(void* item, byte value);
	int ConfigLength();
	int ComObjCount();

protected:
	ARMPinConfig* config;
	int port;
	GenericPin *pin;
};

#endif /* INPUTITEM_H_ */
