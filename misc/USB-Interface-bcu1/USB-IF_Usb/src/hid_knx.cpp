/*
 *  hid_knx.cpp - Processing of the HID packets
 *
 *  Copyright (C) 2018-2021 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <GenFifo.h>
#include "stdio.h"
#include "chip.h"
#include "hid_knx.h"
#include "string.h"
#include "busdevice_if.h"
#include "BufferMgr.h"
#include "tel_dump_usb.h"
#include "device_mgnt.h"
#include "GenFifo.h"
#include "error_handler.h"

KnxHidIf knxhidif;

// Called on HID Get Report Request
ErrorCode_t HidIf_GetReport(USBD_HANDLE_T hHid, USB_SETUP_PACKET *pSetup, uint8_t * *pBuffer, uint16_t *plength)
{
  // No response to GetReport
  return ERR_USBD_STALL;
}

// Called on HID Set Report Request
ErrorCode_t HidIf_SetReport(USBD_HANDLE_T hHid, USB_SETUP_PACKET *pSetup, uint8_t * *pBuffer, uint16_t length)
{
  // No response to SetReport
  return ERR_USBD_STALL;
}

// HID interrupt endpoint handler
ErrorCode_t HidIf_Ep_Hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	return knxhidif.Hdlr(event);
}

ErrorCode_t KnxHidIf::Hdlr(uint32_t event)
{
	switch (event) {
	case USB_EVT_IN: // Completed sending IN packet from device to host
		tx_busy = false;
		break;
	case USB_EVT_OUT: // Completed receiving OUT packet from host to device
		if (rx_avail < 255)
			rx_avail++; // keine Ahnung, ob mehrere Pakete gepuffert werden koennen...
	}
	return LPC_OK;
}

void KnxHidIf::Set_hUsb(USBD_HANDLE_T h_Usb)
{
	hUsb = h_Usb;
}

void HidIfSet_hUsb(USBD_HANDLE_T h_Usb)
{
  knxhidif.Set_hUsb(h_Usb);
}

void Split_CdcEnqueue(char* ptr, unsigned len)
{
	while (len)
	{
		unsigned partlen = len;
		if (partlen > 64)
			partlen = 64;
		int buffno = buffmgr.AllocBuffer();
		if (buffno < 0)
			return; // Kein Speicher frei? -> abbrechen
		uint8_t* partptr = buffmgr.buffptr(buffno);
		*partptr++ = partlen+3; // Länge
		*partptr++ = 0; // Checksumme (unbenutzt)
		*partptr++ = 3; // Kennung für CDC (hier eigentlich unnötig)
		memcpy(partptr, ptr, partlen);
		ptr+=partlen;
		len-=partlen;
  	if (cdc_txfifo.Push(buffno) != TFifoErr::Ok)
  	{
  		int no;
  		while (cdc_txfifo.Pop(no) == TFifoErr::Ok)
  		{
  			buffmgr.FreeBuffer(no);
  		}
  	}
	}
}

void DumpReport2Cdc(bool DirSend, uint8_t* data)
{
	if (CdcDeviceMode == TCdcDeviceMode::UsbMon)
	{
		static uint16_t SeqNo = 0;
		unsigned len;
		char line[210];
		//char part[10];
		char *ptr;
		if (DirSend)
			snprintf(line, sizeof(line), "%04u IN  ", SeqNo++);
		else
			snprintf(line, sizeof(line), "%04u OUT ", SeqNo++);
		if (SeqNo >= 10000)
			SeqNo = 0;
		len = data[IDX_HRH_DataLen];
		if ((len <= 8) || (len+3 > HID_REPORT_SIZE))
		{
			// Längenangabe unplausibel, jetzt wird die Länge bestimmt,
			// indem von hinten das erste nicht-Nullbyte gesucht wird.
			// Es wird mindestens 1 Byte ausgegeben.
			for (len=63; (len>1) && (data[len] == 0); len--);
		} else {
			len += 3;
		}
		ptr = &line[9];
		for (unsigned i=0; i<len; i++)
		{
			sprintf(ptr, "%02X ", *data++);
			ptr+=3;
		}
		*ptr++ = 13;
		*ptr++ = 10;
		*ptr = 0;
		// Und jetzt den evtl. langen String in 64 Byte Häppchen splitten und im Cdc-Fifo einreihen
		len = strlen(line);
		ptr = &line[0];
		Split_CdcEnqueue(ptr, len);
	}
}

ErrorCode_t KnxHidIf::SendReport(uint8_t* data)
{
  unsigned int startTime = systemTime;
  while (tx_busy)
  {
    if (systemTime > (startTime+10))
      return ERR_TIME_OUT;
  }
  tx_busy = true;
  if (USBD_API->hw->WriteEP(hUsb, HID_EP_IN, data, HID_REPORT_SIZE) != HID_REPORT_SIZE)
    return ERR_FAILED;
  else {
    DumpReport2Cdc(true, data);
    return LPC_OK;
  }
}

ErrorCode_t KnxHidIf::ReadAvail(void)
{
	if (rx_avail)
		return LPC_OK;
	return ERR_FAILED;
}

ErrorCode_t KnxHidIf::ReadReport(int &buffno)
{
	if (ReadAvail() == LPC_OK)
	{
		buffno = buffmgr.AllocBuffer();
		if (buffno < 0)
			return ERR_FAILED;
		uint8_t* ptr = buffmgr.buffptr(buffno)+2;
		NVIC_DisableIRQ(USB0_IRQn);
		rx_avail--;
		NVIC_EnableIRQ(USB0_IRQn);
		unsigned len = USBD_API->hw->ReadEP(hUsb, HID_EP_OUT, ptr);
		if (len != HID_REPORT_SIZE)
		{
			buffmgr.FreeBuffer(buffno);
			buffno = -1;
			return ERR_FAILED;
		} else {
			DumpReport2Cdc(false, ptr);
			return LPC_OK;
		}
	} else
		return ERR_FAILED;
}

/* Baut ein Paket auf zum Versand ueber USB an den PC.
 * Die Daten werden in ToSendDataBuffer abgelegt.
 * Dabei werden die Defaults fuer dieses System angenommen,
 * EMI-Type, Versionsnummern und Co.
 * Parameter
 * ProtId: Protocol Identifier
 *          - TPH_ProtocolID::knxTunnel
 *          - TPH_ProtocolID::busAccessServer
 * PayloadLen: Payload Length, Laenge des Transfer Protocol Body
 * EmiServiceId:
 *          - Service Identifier bei TPH_ProtocolID::busAccessServer
 *          - EMI ID bei TPH_ProtocolID::knxTunnel
 * Der zurueckgegebene Pointer zeigt auf das erste Byte des
 * KNX Transfer Protocol Body.
 */
uint8_t* KnxHidIf::BuildUsbPacket(uint8_t *ptr, uint8_t ProtId, uint8_t PayloadLen, BAS_ServiceId EmiServiceId)
{
  *ptr++ = 0x01; // Report Identifier
  *ptr++ = 0x13; // Packet Info - nur Single Packets unterstuetzt im Moment
  *ptr++ = TPH_ProtocolLength_V0+PayloadLen;
  *ptr++ = 0;
  *ptr++ = TPH_ProtocolLength_V0;
  *ptr++ = 0;
  *ptr++ = PayloadLen;
  *ptr++ = ProtId;
  *ptr++ = static_cast<uint8_t>(EmiServiceId);
  *ptr++ = TPH_ManufacturerCode_V0_HIGH_BYTE;
  *ptr++ = TPH_ManufacturerCode_V0_LOW_BYTE;
  return ptr;
}

void KnxHidIf::ReceivedUsbBasPacket(BAS_ServiceId ServiceId, unsigned BodyLen, uint8_t* Buffer)
{
  BAS_FeatureId Feature = static_cast<BAS_FeatureId>(Buffer[IDX_TPB_FeatureId]);
  uint8_t TxBuffer[HID_REPORT_SIZE];

  // Only the BAS (i.e. we) may send a FeatureInfo if it detects a bus status change
  if ((ServiceId == BAS_ServiceId::FeatureInfo) || (ServiceId == BAS_ServiceId::FeatureResp))
  {
    failHardInDebug();
    return;
  }

  // Only set of ActiveEmi allowed, all others are get only. KNX Spec. 9.3 / 3.5.3.3.2 -> 3.5.3.4.2
  if ((Feature != BAS_FeatureId::ActiveEmi) && (ServiceId == BAS_ServiceId::FeatureSet))
  {
    failHardInDebug();
    return;
  }

  uint8_t* ptr = nullptr;
  switch (Feature)
  {
  case BAS_FeatureId::SuppEmiType:
    ptr = BuildUsbPacket(TxBuffer, TPH_ProtocolID::busAccessServer, 3, BAS_ServiceId::FeatureResp);
    *ptr++ = static_cast<uint8_t>(BAS_FeatureId::SuppEmiType);
    *ptr++ = 0; //
    *ptr++ = 1; // erst mal nur EMI 1
    SendReport(TxBuffer);
    break;

  case BAS_FeatureId::DescrType0: // auch Mask-Version genannt
    ptr = BuildUsbPacket(TxBuffer, TPH_ProtocolID::busAccessServer, 3, BAS_ServiceId::FeatureResp);
    *ptr++ = static_cast<uint8_t>(BAS_FeatureId::DescrType0);
    *ptr++ = 0x00; // BCU 1, Subcode 0
    *ptr++ = 0x10; // Vorbild antwortet hier 0x12 !
    SendReport(TxBuffer);
    break;

  case BAS_FeatureId::BusConnStat:
    ptr = BuildUsbPacket(TxBuffer, TPH_ProtocolID::busAccessServer, 2, BAS_ServiceId::FeatureResp);
    *ptr++ = static_cast<uint8_t>(BAS_FeatureId::BusConnStat);
    *ptr++ = (devicemgnt.KnxIsActive()) ? 1:0;
    SendReport(TxBuffer);
    break;

  case BAS_FeatureId::KnxManCode:
    ptr = BuildUsbPacket(TxBuffer, TPH_ProtocolID::busAccessServer, 3, BAS_ServiceId::FeatureResp);
    *ptr++ = static_cast<uint8_t>(BAS_FeatureId::KnxManCode);
    *ptr++ = C_ManufacturerCodeHigh;
    *ptr++ = C_ManufacturerCodeLow;
    SendReport(TxBuffer);
    break;

  case BAS_FeatureId::ActiveEmi:
    // Set has only one valid value, as only EMI1 is currently supported
    if (ServiceId == BAS_ServiceId::FeatureGet)
    {
      ptr = BuildUsbPacket(TxBuffer, TPH_ProtocolID::busAccessServer, 2, BAS_ServiceId::FeatureResp);
      *ptr++ = static_cast<uint8_t>(BAS_FeatureId::ActiveEmi);
      *ptr++ = 1; // EMI 1
      SendReport(TxBuffer);
      return;
    }

    if ((ServiceId == BAS_ServiceId::FeatureSet) && (BodyLen == 2))
    {
      uint8_t * data = &Buffer[IDX_TPB_FeatureData];
      uint16_t emiToSet = (data[1] << 8) | data[0];
      if (emiToSet == 1) // only EMI 1 implemented
      {
        return;
      }
    }
    failHardInDebug();
    return;
    break;

  default:
      failHardInDebug();
    break; // Unbekannt, weg hier...
  }
}

void KnxHidIf::ReceivedUsbPacket(int buffno)
{
  uint8_t* Buffer = buffmgr.buffptr(buffno)+2;
  // Check HID Report Header
  unsigned ReportPacketLength = Buffer[IDX_HRH_DataLen];
  if ((Buffer[IDX_HRH_Id] == C_HRH_IdHid) &&
      (Buffer[IDX_HRH_PkInfo] == C_HRH_PacketInfoSinglePacket) &&
      (ReportPacketLength > TPH_ProtocolLength_V0) &&
      ((ReportPacketLength+3) <= 64))
  {
    *(Buffer-2) = ReportPacketLength+C_HRH_HeadLen+2;
    Buffer+=C_HRH_HeadLen;
    // Buffer now points to the HID Report Body / Transfer Protocol Header
    unsigned TransferBodyLength = (Buffer[IDX_TPH_BodyLen] << 8) + Buffer[IDX_TPH_BodyLen+1];
    if ((Buffer[IDX_TPH_Version] == TPH_ProtocolVersion_V0) &&
        (Buffer[IDX_TPH_HeadLen] == TPH_ProtocolLength_V0) &&
        (ReportPacketLength == (TransferBodyLength+TPH_ProtocolLength_V0)) &&
        (TransferBodyLength >= 1) &&
        (Buffer[IDX_TPH_ManuCode1] == TPH_ManufacturerCode_V0_HIGH_BYTE) &&
        (Buffer[IDX_TPH_ManuCode2] == TPH_ManufacturerCode_V0_LOW_BYTE))
    {
      switch (Buffer[IDX_TPH_ProtId])
      {
      case TPH_ProtocolID::knxTunnel:
        // diese Pakete werden alle weitergeleitet
        if (!deviceIf.Hid2Knx_Ena()) // die Überprüfung hier könnte entfallen, muss der Dispatcher eh machen
        {
            failHardInDebug();
            return;
        }

        if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
        {
          buffmgr.FreeBuffer(buffno);
          failHardInDebug();
        }
        return; // Damit wird der Buffer weiter unten nicht freigegeben.
        break;
      case TPH_ProtocolID::busAccessServer:
        ReceivedUsbBasPacket(static_cast<BAS_ServiceId>(Buffer[IDX_TPH_SerId]), TransferBodyLength,
                Buffer+TPH_ProtocolLength_V0);
        break;

      default:
        ; // Irgendwas anderes, weg hier...
        failHardInDebug();
      }
    }
    else
    {
        failHardInDebug();
    }
  }
  else
  {
      failHardInDebug();
  }
  buffmgr.FreeBuffer(buffno);
}

KnxHidIf::KnxHidIf(void)
{
  tx_busy = false;
  rx_avail = 0;
}

bool KnxHidIf::UsbIsConfigured(void)
{
	return USB_IsConfigured(hUsb);
}

void KnxHidIf::KnxIf_Tasks(void)
{
	int buffno;
	if ( USB_IsConfigured(hUsb)) {
		if (ReadReport(buffno) == LPC_OK)
		{
			ReceivedUsbPacket(buffno);
			deviceIf.BlinkActivityLed();
		}
	} else {
		tx_busy = false;
		rx_avail = 0;
	}

	if ( USB_IsConfigured(hUsb))
	{
		if ((!tx_busy) && (hid_txfifo.Empty() != TFifoErr::Empty))
		{
			int buffno;
			hid_txfifo.Pop(buffno);
			uint8_t *ptr = buffmgr.buffptr(buffno);
			if (((ptr[C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_MCode+2] & C_MCode_USB_IF_SpecialMask) == 0) ||
					(ptr[C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_MCode+2] == 0xA0))
			{ // Nur wenn kein "Spezial-MCode" (selber definierte Pakete)
				// A0, die Antwort auf einen EMI Reset-Request, muss allerdings auch
				// über USB weitergeschickt werden. Da die Monitorfunktion intern nie
			  // einen Reset erzeugt, ist das unproblematisch.
				SendReport(&ptr[2]);
				deviceIf.BlinkActivityLed();
			}

			if (CdcDeviceMode == TCdcDeviceMode::BusMon)
			{ // Nur im Monitor-Mode Telegramme über CDC im Klartext ausgeben
				bool mon = false;
				bool send = false;
				if ((ptr[C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_MCode+2] & C_MCode_USB_IF_MonitorMask) == (C_MCode_TxEcho & C_MCode_USB_IF_MonitorMask))
				{
					mon = true;
					send = true;
				}
				if ((ptr[C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_MCode+2] & C_MCode_USB_IF_MonitorMask) == (C_MCode_RxData & C_MCode_USB_IF_MonitorMask))
				{
					mon = true;
				}
				if (mon)
				{
					unsigned telLength = ptr[0];
					if ((telLength > 2) && (telLength < 66))
					{
						teldump.Dump(systemTime, send, telLength-(2+C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_Data), ptr+2+C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_Data);
					}
				}
			}
			buffmgr.FreeBuffer(buffno);
		}
	} else {
		int buffno;
		while (hid_txfifo.Empty() != TFifoErr::Empty)
		{
			hid_txfifo.Pop(buffno);
			buffmgr.FreeBuffer(buffno);
		}
	}
}

