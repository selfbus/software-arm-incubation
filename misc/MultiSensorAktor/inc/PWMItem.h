/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PWMITEM_H_
#define PWMITEM_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/timer.h>
#include <GenericItem.h>
#include <PortConfig.h>

class PWMItem : public GenericItem
{
public:
	PWMItem(BcuBase* bcu, int port, int firstComIndex, PortConfig* config, GenericItem* nextItem);
	~PWMItem() = default;

	virtual void Loop(int updatedObjectNo);

	PWMItem *NextInstance;
	void IRQ();

protected:
	byte value;
	Timer timer = timer16_0;
};

#endif /* PWMITEM_H_ */
