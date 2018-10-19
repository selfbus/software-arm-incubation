/*
 *  tel_dump.h - Printing KNX packets as debug output
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef TEL_DUMP_H_
#define TEL_DUMP_H_

class TelDump
{
public:
	int Dump(bool DirSend, uint8_t* telptr);
protected:
	unsigned linelen;
	unsigned remlen;
	char* linebuffer;
	void buf_printf(const char *fmt, ...);
	void PrintPhyAddr(uint8_t dat0, uint8_t dat1);
	void PrintPacketNo( unsigned no );
	void DumpHexData(uint8_t tel[], unsigned int start, unsigned int len);
	void DumpMemData(uint8_t tel[], unsigned int len, unsigned ExtendedOffset, unsigned ItsUserMem = 0);
	void DumpPropValHeader(uint8_t tel[], unsigned int ExDReq);
	void DbgParseTele(uint8_t tel[], unsigned int len);
};

extern TelDump teldump;

void Split_CdcEnqueue(char* ptr, unsigned len);

#endif /* TEL_DUMP_H_ */
