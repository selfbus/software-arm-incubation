/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SHT2XITEM_H_
#define SHT2XITEM_H_

#include <sblib/eib/bcu_base.h>
#include <GenericItem.h>
#include <SHT2xConfig.h>
#include <sblib/i2c/SHT2x.h>

class SHT2xItem : public GenericItem
{
public:
	SHT2xItem(BcuBase* bcu, byte firstComIndex, SHT2xConfig* config, GenericItem* nextItem);
	~SHT2xItem() = default;

	void Loop(uint32_t now, int updatedObjectNo);
	int ConfigLength() { return sizeof(SHT2xConfig); }
	int ComObjCount() { return 4; }

protected:
	SHT2xConfig *config;
    SHT2xClass sht2x;
	uint32_t nextAction = 0;
	byte state = 0;
	float offset;
};

#endif /* SHT2XITEM_H_ */
