/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef OUTPUTITEM_H_
#define OUTPUTITEM_H_

#include <sblib/eib/bcu_base.h>
#include <GenericItem.h>
#include <PortConfig.h>

class OutputItem : public GenericItem
{
public:
	OutputItem(BcuBase* bcu, int port, int firstComIndex, PortConfig* config, GenericItem* nextItem);
	~OutputItem() = default;

	virtual void Loop(int updatedObjectNo);

protected:
	unsigned int blinkActionTime = 0xFFFFFFFF;
};

#endif /* OUTPUTITEM_H_ */
