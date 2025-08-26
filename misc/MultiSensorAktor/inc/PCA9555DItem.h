/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PCA9555DITEM_H_
#define PCA9555DITEM_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/debounce.h>

#include <GenericItem.h>
#include <GenericPin.h>
#include <PCA9555DConfig.h>

class PCA9555DItem : public GenericItem
{
public:
	PCA9555DItem(byte firstComIndex, PCA9555DConfig* config, GenericItem* nextItem, uint16_t& objRamPointer);
	~PCA9555DItem() = default;

	void Loop(uint32_t now, int updatedObjectNo);
	int ConfigLength() { return configLength; }
	int ComObjCount() { return comObjCount; }

protected:
	void initPCA();

	PCA9555DConfig *config;
	uint16_t inOutConfig = 0;
	bool configured = 0;
	//uint16_t lockFlag = 0;
	uint16_t outFlag = 0;
	uint32_t checkTime = 0;
	uint32_t forceCheckTill = 0;

	GenericPin* pins[16];

	byte configLength;
	byte comObjCount;
};

#endif /* PCA9555DITEM_H_ */
