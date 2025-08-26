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
#include <GenericPin.h>
#include <ARMPinConfig.h>

class ARMPinItem : public GenericItem
{
public:
	ARMPinItem(byte firstComIndex, byte portNumber, ARMPinConfig* config, GenericItem* nextItem, uint16_t	& objRamPointer);
	~ARMPinItem() = default;

	void Loop(uint32_t now, int updatedObjectNo);
//	void Irq(void* item, byte value);
	int ConfigLength();
	int ComObjCount();

	static const constexpr int PortPins[] = {
			PIO0_2, PIO0_3, PIO0_4, PIO0_5, PIO0_6, PIO0_7, PIO0_8, PIO0_9, PIO0_11,
			PIO1_0, PIO1_1, PIO1_2, PIO1_5, PIO1_10,
			PIO2_0, PIO2_1, PIO2_2, PIO2_3, PIO2_4, PIO2_5, PIO2_6, PIO2_7, PIO2_8, PIO2_9, PIO2_10, PIO2_11,
			PIO3_0, PIO3_1, PIO3_2, PIO3_3, PIO3_4, PIO3_5,
			PIO1_4, PIO1_6, PIO1_7, PIO1_11
	};

protected:
	ARMPinConfig* config;
	int port;
	GenericPin *pin;
};

#endif /* INPUTITEM_H_ */
