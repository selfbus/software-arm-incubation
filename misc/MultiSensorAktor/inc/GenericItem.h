/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef GENERICITEM_H_
#define GENERICITEM_H_

#include <PortConfig.h>

class GenericItem
{
public:
	GenericItem(BcuBase* bcu, int port, int firstComIndex, PortConfig* config, GenericItem* nextItem) : nextItem(nextItem), bcu(bcu), port(port), firstComIndex(firstComIndex), config(config) {};

	virtual void Loop(int updatedObjNo) = 0;
	GenericItem* GetNextItem() { return nextItem; };

protected:
	GenericItem* nextItem;
	BcuBase* bcu;
	int port;
	int firstComIndex;
	PortConfig* config;
};

#endif /* GENERICITEM_H_ */
