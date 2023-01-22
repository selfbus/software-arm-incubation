/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef INPUTITEM_H_
#define INPUTITEM_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/ioports.h>
#include <sblib/debounce.h>
#include <GenericItem.h>
#include <ARMPinConfig.h>
#include <GenericPin.h>

class ARMPinItem : public GenericItem
{
public:
	ARMPinItem(BcuBase* bcu, byte firstComIndex, byte portNumber, ARMPinConfig* config, GenericItem* nextItem, uint16_t& objRamPointer);
	~ARMPinItem() = default;

	void Loop(uint32_t now, int updatedObjectNo);
	void Irq(void* item, byte value);
	int ConfigLength();
	int ComObjCount();

	const static int PortPins[];

protected:
	ARMPinConfig* config;
	int port;
	GenericPin *pin;
};

#endif /* INPUTITEM_H_ */
