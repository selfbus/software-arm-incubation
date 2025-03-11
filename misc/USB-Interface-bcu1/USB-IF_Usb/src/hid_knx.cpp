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
    switch (event)
    {
        case USB_EVT_IN: // Completed sending IN packet from device to host
            tx_busy = false;
            break;

        case USB_EVT_OUT: // Completed receiving OUT packet from host to device
            if (rx_avail < 255)
                rx_avail++; // keine Ahnung, ob mehrere Pakete gepuffert werden koennen...
            break;

        default:
            failHardInDebug();

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

void flushBuffer(GenFifo<int> buffer)
{
    int no;
    while (buffer.Pop(no) == TFifoErr::Ok)
    {
        buffmgr.FreeBuffer(no);
    }
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
        {
            // We will always come here, then nobody is receiving our USB CDC packets.
            // E.g. We are successfully connected by USB to a PC, but no program/terminal
            // has opened the CDC virtual com port to receive our packets.
            flushBuffer(cdc_txfifo);
            return; // Kein Speicher frei? -> abbrechen
        }
        uint8_t* partptr = buffmgr.buffptr(buffno);
        *partptr++ = partlen+3; // Länge
        *partptr++ = 0; // Checksumme (unbenutzt)
        *partptr++ = 3; // Kennung für CDC (hier eigentlich unnötig)
        memcpy(partptr, ptr, partlen);
        ptr+=partlen;
        len-=partlen;
        if (cdc_txfifo.Push(buffno) != TFifoErr::Ok)
        {
            flushBuffer(cdc_txfifo);
        }
    }
}

void DumpReport2Cdc(bool isIncomingData, uint8_t* data)
{
    if (currentDeviceMode != DeviceMode::UsbMon)
    {
        return;
    }
    static uint16_t seqNo = 0;
    char line[210];
    constexpr size_t lineLength = sizeof(line)/sizeof(line[0]);
    char * ptrCurrentLinePosition = &line[0];
    if (isIncomingData)
    {
        snprintf(line, lineLength, "%04u IN  ", seqNo);
        ptrCurrentLinePosition += 9; // move same amount as above´s snprintf output
    }
    else
    {
        snprintf(line, lineLength, "%04u OUT ", seqNo);
        ptrCurrentLinePosition += 9; // move same amount as above´s snprintf output
    }

    seqNo++;
    if (seqNo >= 10000)
    {
        seqNo = 0;
    }

    uint8_t len = data[IDX_HRH_DataLen];
    if ((len <= 8) || (len + 3 > HID_REPORT_SIZE))
    {
        ///\todo this "reversed 0x00 check" will most likely fail, because uint8_t* data may not be fully initialized with zeros.
        // Längenangabe unplausibel, jetzt wird die Länge bestimmt,
        // indem von hinten das erste nicht-Nullbyte gesucht wird.
        // Es wird mindestens 1 Byte ausgegeben.
        for (len = 63; (len > 1) && (data[len] == 0); len--)
        {
            ;
        }
        failHardInDebug();
    }
    else
    {
        len += 3;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        sprintf(ptrCurrentLinePosition, "%02X ", *data++);
        ptrCurrentLinePosition += 3; // move same amount as above´s snprintf output
    }
    *ptrCurrentLinePosition++ = 13; // carrige return
    *ptrCurrentLinePosition++ = 10; // line feed
    *ptrCurrentLinePosition = 0; // zero-terminated string
    // Und jetzt den evtl. langen String in 64 Byte Häppchen splitten und im Cdc-Fifo einreihen
    len = strlen(line);
    Split_CdcEnqueue(&line[0], len);
}

void KnxHidIf::resetRx()
{
    rx_avail = 0;
}

void KnxHidIf::resetTx()
{
    tx_busy = false;
    lastSysTickSendReport = 0;
}

void KnxHidIf::reset()
{
    resetRx();
    resetTx();
}

ErrorCode_t KnxHidIf::SendReport(uint8_t* data)
{
    if (!USB_IsConfigured(hUsb))
    {
        reset();
        return ERR_FAILED;
    }

    uint32_t startTime = systemTime;
    while (tx_busy)
    {
        if ((systemTime - lastSysTickSendReport) > KnxHidIf::HidTxTimeoutMs)
        {
            resetTx();
            break;
        }

        if (systemTime > (startTime + KnxHidIf::HidTxBusyWaitMs))
        {
            failHardInDebug();
            return ERR_TIME_OUT;
        }
    }

    tx_busy = true;
    if (USBD_API->hw->WriteEP(hUsb, HID_EP_IN, data, HID_REPORT_SIZE) != HID_REPORT_SIZE)
    {
        failHardInDebug();
        return ERR_FAILED;
    }
    else
    {
        lastSysTickSendReport = systemTime;
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
    if (ReadAvail() != LPC_OK)
    {
        return ERR_FAILED;
    }

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
    *ptr++ = C_HRH_IdHid; // Report Identifier
    *ptr++ = C_HRH_PacketInfoSinglePacket; // Packet Info - nur Single Packets unterstuetzt im Moment
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

    uint8_t TxBuffer[HID_REPORT_SIZE];
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
            *ptr++ = (devicemgnt.getKnxActive()) ? 1:0;
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
}

bool KnxHidIf::UsbIsConfigured(void)
{
    return USB_IsConfigured(hUsb);
}

void KnxHidIf::KnxIf_Tasks(void)
{
    if (!USB_IsConfigured(hUsb))
    {
        // USB not configured => clear pending tx buffers
        reset();
        while (hid_txfifo.Empty() != TFifoErr::Empty)
        {
            int dummy;
            hid_txfifo.Pop(dummy);
            buffmgr.FreeBuffer(dummy);
        }
        return;
    }

    // Check for incoming HID reports
    int rxBuffNo;
    if (ReadReport(rxBuffNo) == LPC_OK)
    {
        ReceivedUsbPacket(rxBuffNo);
        deviceIf.BlinkActivityLed();
    }

    // Check for pending outgoing HID reports
    if (tx_busy)
    {
        return;
    }

    // Check if KNX connection state has changed
    if (hid_txfifo.Empty() == TFifoErr::Empty)
    {
        if (lastKnxState != devicemgnt.getKnxActive())
        {
            setLastKnxState(devicemgnt.getKnxActive());
        }
        return;
    }

    // Send outgoing HID reports
    int txBuffNo;
    hid_txfifo.Pop(txBuffNo);
    uint8_t *ptr = buffmgr.buffptr(txBuffNo);
    uint8_t emiMessageCode = ptr[C_HRH_HeadLen + TPH_ProtocolLength_V0 + IDX_TPB_MCode + 2];
    if ((emiMessageCode & C_MCode_USB_IF_SpecialMask) == 0)
    {
        // handle standard conform EMI 1 telegram
        // Nur wenn kein "Spezial-MCode" (selber definierte Pakete)
        SendReport(&ptr[2]);
        deviceIf.BlinkActivityLed();
    }
    else if (emiMessageCode == C_MCode_PEI_Reset)
    {
        SendReport(&ptr[2]);
        deviceIf.BlinkActivityLed();
    }
    else
    {
        // handle USB-IF masked EMI 1 telegrams (most likely C_MCode_TxEcho / C_MCode_RxData)
        handleBusMonitorMode(ptr);
    }
    buffmgr.FreeBuffer(txBuffNo);
}

void KnxHidIf::setLastKnxState(bool newKnxState)
{
    if (lastKnxState == newKnxState)
    {
        return;
    }
    lastKnxState = newKnxState;
    if (!((devicemgnt.getDeviceMode() == DeviceMode::HidOnly) || (devicemgnt.getDeviceMode() == DeviceMode::UsbMon)))
    {
        return;
    }
    // Send a BAS_ServiceId::FeatureInfo with BAS_FeatureId::BusConnStat
    // to indicate that the KNX bus state has changed
    uint8_t txHidBuffer[HID_REPORT_SIZE];
    uint8_t *ptr;
    ptr = BuildUsbPacket(txHidBuffer, TPH_ProtocolID::busAccessServer, 2,
            BAS_ServiceId::FeatureInfo);
    *ptr++ = static_cast<uint8_t>(BAS_FeatureId::BusConnStat);
    *ptr++ = lastKnxState ? 1 : 0;
    SendReport(txHidBuffer);
}

void KnxHidIf::handleBusMonitorMode(uint8_t * buffer)
{
    // Nur im Monitor-Mode Telegramme über CDC im Klartext ausgeben
    if ((currentDeviceMode != DeviceMode::BusMon) && (currentDeviceMode != DeviceMode::UsbMon))
    {
        return;
    }

    // get EMI message code
    uint8_t emiMessageCode = buffer[C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_MCode+2];
    // "unmasked" USB-IF special EMI message code
    emiMessageCode = emiMessageCode & C_MCode_USB_IF_MonitorMask;

    // Check for USB-IF masked C_MCode_TxEcho
    bool isTxTelegram = C_MCode_TxEcho == emiMessageCode;
    // Check for USB-IF masked C_MCode_RxData
    bool isRxTelegram = C_MCode_RxData == emiMessageCode;

    if (!isTxTelegram && !isRxTelegram)
    {
        failHardInDebug();
        return;
    }

    uint32_t telLength = buffer[0];
    if ((telLength <= 2) || (telLength >= 66))
    {
        failHardInDebug();
        return;
    }

    switch (currentDeviceMode)
    {
        case DeviceMode::UsbMon:
            DumpReport2Cdc(isTxTelegram, buffer + 2);
            break;
        case DeviceMode::BusMon:
            // Dump telegram
            teldump.Dump(systemTime, isTxTelegram, telLength-(2+C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_Data),
                        buffer+2+C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_Data);
            break;
        default:
            failHardInDebug(); // This should never happen
            break;
    }
}
