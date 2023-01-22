/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "HelperFunctions.h"

void HelperFunctions::setComObjPtr(BcuBase* bcu, int objno, ComType type, uint16_t &newAddr)
{
	const byte* configTable = bcu->comObjects->objectConfigTable();
	if (configTable != nullptr)
	{
		int size = 1;
	    if (type > BYTE_1)
	    {
	    	const byte objectTypeSizes[10] = { 1, 1, 2, 3, 4, 6, 8, 10, 14, 14 };
	    	size = objectTypeSizes[type - BIT_7];
	    }

	    if (newAddr + size > 0x5FC + 0x60 && newAddr < 0x6C5)
	    {
	    	newAddr = 0x6C5 + 128;
	    }

	    uint16_t offSet = 1 + sizeof(ComConfigBCU2::DataPtrType) + objno * sizeof(ComConfigBCU2);
		ComConfigBCU2* cfg = (ComConfigBCU2*) (configTable + offSet);
		cfg->baseConfig.type = type;

	    byte * addr = (byte *) &cfg->dataPtr;
	    if (false)
	    {
	    	addr[1] = (newAddr >> 8) & 0xFF;
	    	addr[0] = newAddr & 0xFF;
	    }
	    else
	    {
	    	addr[0] = (newAddr >> 8) & 0xFF;
	    	addr[1] = newAddr & 0xFF;
	    }

	    newAddr += size;
	}
}

void HelperFunctions::setFlagsTablePtr(BcuBase* bcu, uint16_t flagsAddr)
{
	byte* configTable = bcu->comObjects->objectConfigTable();
	if (configTable != nullptr)
	{
	    if (false)
	    {
	    	configTable[2] = (flagsAddr >> 8) & 0xFF;
	    	configTable[1] = flagsAddr & 0xFF;
	    }
	    else
	    {
	    	configTable[1] = (flagsAddr >> 8) & 0xFF;
	    	configTable[2] = flagsAddr & 0xFF;
	    }
	}
}
