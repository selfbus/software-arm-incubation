/*
 *  hid_knx.h - Processing of the HID packets
 *
 *  Copyright (C) 2018-2021 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef INC_HID_KNX_H_
#define INC_HID_KNX_H_

#include "usbd_rom_api.h"

#define HID_REPORT_SIZE        64

#ifdef __cplusplus

#include "knxusb_const.h"

class KnxHidIf
{
public:
  KnxHidIf(void);

	ErrorCode_t Hdlr(uint32_t event);
	void Set_hUsb(USBD_HANDLE_T h_Usb);

	void KnxIf_Tasks(void);
	bool UsbIsConfigured(void);
protected:
	USBD_HANDLE_T hUsb;	// Handle to USB stack.
	bool tx_busy;
	unsigned rx_avail;
	void ReceivedUsbBasPacket(BAS_ServiceId ServiceId, unsigned BodyLen, uint8_t* Buffer);
	void ReceivedUsbPacket(int buffno);
	uint8_t* BuildUsbPacket(uint8_t *ptr, uint8_t ProtId, uint8_t PayloadLen, BAS_ServiceId EmiServiceId);
	ErrorCode_t SendReport(uint8_t* data);
	ErrorCode_t ReadReport(int &buffno);
	ErrorCode_t ReadAvail(void);

private:
    void handleBusMonitorMode(uint8_t * buffer);

};

extern KnxHidIf knxhidif;

void Split_CdcEnqueue(char* ptr, unsigned len);

extern "C"
{
#endif

ErrorCode_t HidIf_Ep_Hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event);

ErrorCode_t HidIf_GetReport(USBD_HANDLE_T hHid, USB_SETUP_PACKET *pSetup, uint8_t * *pBuffer, uint16_t *plength);

ErrorCode_t HidIf_SetReport(USBD_HANDLE_T hHid, USB_SETUP_PACKET *pSetup, uint8_t * *pBuffer, uint16_t length);

void HidIfSet_hUsb(USBD_HANDLE_T h_Usb);

#ifdef __cplusplus
}
#endif

#endif /* INC_HID_KNX_H_ */
