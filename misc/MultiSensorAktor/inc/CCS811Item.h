/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CCS811ITEM_H_
#define CCS811ITEM_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/debounce.h>
#include <GenericItem.h>
#include <CCS811Config.h>
#include <sblib/i2c/CCS811.h>

class CCS811Item : public GenericItem
{
public:
	CCS811Item(BcuBase* bcu, byte firstComIndex, CCS811Config* config, GenericItem* nextItem);
	~CCS811Item() = default;

	void Loop(uint32_t now, int updatedObjectNo);
	int ConfigLength() { return sizeof(CCS811Config); }
	int ComObjCount() { return 5; }

protected:
	CCS811Config *config;
	uint32_t nextAction = 0;
	byte state = 0;
	bool configured = false;
	CCS811Class ccs811;
};

#endif /* CCS811ITEM_H_ */
