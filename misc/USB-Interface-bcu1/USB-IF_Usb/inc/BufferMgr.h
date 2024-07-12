/*
 *  BufferMgr.h - Manages a number of fixed size buffers
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef BUFFERMGR_H_
#define BUFFERMGR_H_

#include <stdint.h>

#define BUFF_CNT 8
#define BUFF_SIZE 68
/*
 * Aufbau eines Pakets:
 * 1 Byte Paketlänge, über alle Bytes gezählt
 * 1 Byte Checksumme, alle Bytes des Telegrams addiert (incl Checksumme), modulo 256, ergeben 255
 * Bei einem HID-Paket folgt hier der bis zu 64 Byte lange HID-Report, incl. Report Nummer 1 am Anfang.
 * Bei einem CDC-Paket folgt als Byte eine 2 zur Unterscheidung von einem HID-Report. Danach bis zu 64
 * Byte Nutzdaten.
 * Diese Pakete werden im paketorientierten Modus genau so über die serielle Schnittstelle gesendet
 * und empfangen.
 * Auch im RawMode der seriellen Schnittstelle wird dieser Aufbau intern beibehalten, der
 * Uart-Transceiver strippt die drei Header-Bytes jedoch vor dem Versenden bzw. ergänzt sie nach Empfang.
 */

class BufferMgr
{
public:
	BufferMgr(void);
	void Purge(void);
	int AllocBuffer(void);
	int FreeBuffer(int no);
	uint8_t* buffptr(int no);
protected:
	uint8_t data[BUFF_CNT][BUFF_SIZE] = {0};
	bool alloctable[BUFF_CNT] = {false};
};

extern BufferMgr buffmgr;

#endif /* BUFFERMGR_H_ */
