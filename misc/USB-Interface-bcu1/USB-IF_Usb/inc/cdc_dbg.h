/*
 *  cdc_dbg.h - CDC USB serial emulator: Sending and receiving in buffers
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CDC_DBG_H_
#define CDC_DBG_H_

#include "usbd_rom_api.h"

#ifdef __cplusplus

#define CDC2UARTMAXWAIT 100

class CdcDbgIf
{
public:
	CdcDbgIf(void);
	void Set_hUsb(USBD_HANDLE_T h_Usb);
	void Reset(void);
	void PurgeRx(void);
	void reEnableReceive(void);
	void DbgIf_Tasks(void);
protected:
	USBD_HANDLE_T hUsb;	// Handle to USB stack.
	bool ReceiveEna;
	bool zlp;
	uint8_t CtrlLines;
	unsigned int RecDisTime;

private:
	uint32_t rxByteCounter;
	uint32_t txByteCounter;
	void receiveAndPushToUart();
};

extern CdcDbgIf cdcdbgif;

extern "C"
{
#endif

void CdcIfSet_hUsb(USBD_HANDLE_T h_Usb);

#ifdef __cplusplus
}
#endif


#endif /* CDC_DBG_H_ */
