/*
 *  KnxUsb_Const.h - Offsets and Data of the HID-Packets
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef _KNXUSB_CONST_H
#define _KNXUSB_CONST_H

#include <stdint.h>

#define HID_REPORT_SIZE 64

/**
 * HID Report header report id
 * @details report id is always 1
 * @note KNX Spec. 2.1 9/3 3.4.1.2.1
 */
constexpr uint8_t HRH_ReportID = 0x01;

/**
 * HID Report header packet info sequence number
 * @details Sequence number is always 1 for single packets
 * @note KNX Spec. 2.1 9/3 3.4.1.2.2
 */
constexpr uint8_t HRH_SequenceNumber = 0x01 << 4;

/**
 * HID Report header packet type
 * @note KNX Spec 2.1 9/3 3.4.1.2.3
 */
enum HRH_PacketType
{
    startPacket   = 0x01, //!> 1 = start packet, 0 = not start packet
    endPacket     = 0x02, //!> 1 = end packet, 0 = not end packet
    partialPacket = 0x04, //!> 1 = partial packet, 0 = not partial packet
    notUsed       = 0x08, //!> not used
};

// HID Report Header - HRH
// Die Array-Indexe (IDX_) sind ab Anfang HID Report Header
#define IDX_HRH_Id      0
#define C_HRH_IdHid   1
#define C_HRH_IdCdc   2
#define C_HRH_IdDev   3
#define IDX_HRH_PkInfo  1
/**
 * Single Packet (value 0x13), mehr erst mal nicht unterstuetzen
 */
#define C_HRH_PacketInfoSinglePacket (HRH_SequenceNumber | HRH_PacketType::startPacket | HRH_PacketType::endPacket)
#define IDX_HRH_DataLen 2
#define C_HRH_HeadLen 3

/**
 * Revision of the KNX USB Transfer Protocol
 * @note KNX Spec 2.1 9/3 3.4.1.3.1
 */
constexpr uint8_t TPH_ProtocolVersion_V0 = 0x00;

/**
 * Header length of the KNX USB Transfer Protocol
 * @note KNX Spec 2.1 9/3 3.4.1.3.2
 */
constexpr uint8_t TPH_ProtocolLength_V0 = 0x08;

/**
 * Protocol ID of the KNX USB Transfer Protocol Header
 * @note KNX Spec 2.1 9/3 3.4.1.3.4
 */
enum TPH_ProtocolID
{
    knxTunnel       = 0x01, //!> KNX Tunnel
    mBusTunnel      = 0x02, //!> M-Bus Tunnel (Metering-Bus)
    batiBusTunnel   = 0x03,
    busAccessServer = 0x0f, //!> Bus Access Server Feature Service
};

/**
 * EMI ID of the KNX USB Transfer Protocol Header
 * @note KNX Spec 2.1 9/3 3.4.1.3.4
 */
enum TPH_EMI_ID
{
    EMI1 = 0x01,
    EMI2 = 0x02,
    cEMI = 0x03,
};

/**
 * Manufacturer code of the KNX USB Transfer Protocol
 * @note KNX Spec 2.1 9/3 3.4.1.3.5
 */
constexpr uint8_t TPH_ManufacturerCode_V0_HIGH_BYTE = 0x00;
constexpr uint8_t TPH_ManufacturerCode_V0_LOW_BYTE = 0x00;


// Transfer Protocol Header - TPH
// Die Array-Indexe (IDX_) sind ab Anfang Transfer Protocol Header
#define IDX_TPH_Version 0
#define IDX_TPH_HeadLen 1
#define IDX_TPH_BodyLen 2
#define IDX_TPH_ProtId  4
//#define IDX_TPH_EmiId      5
#define IDX_TPH_SerId      5
////#define C_TPH_SerId 1
#define IDX_TPH_ManuCode1 6
#define IDX_TPH_ManuCode2 7

// Transfer Protocol Body - TPB
#define IDX_TPB_FeatureId   0
#define IDX_TPB_FeatureData 1
#define IDX_TPB_MCode       0
#define IDX_TPB_Data        1 //!< wenn KNX-Frame
#define IDX_TPB_EMI_Len     1 //!< wenn EMI service
#define IDX_TPB_EMI_Addr_h  2
#define IDX_TPB_EMI_Addr_l  3
#define IDX_TPB_EMI_Data    4

/**
 * Device Feature Service of the KNX Bus Access Server - BAS
 * @note KNX Spec 2.1 9/3 3.5.3.2.1
 */
enum class BAS_ServiceId
{
    FeatureGet  = 1, //!> Device Feature Get
    FeatureResp = 2, //!> Device Feature Response
    FeatureSet  = 3, //!> Device Feature Set
    FeatureInfo = 4  //!> Device Feature Info
};

/**
 * Device Feature of the KNX Bus Access Server - BAS
 * @note KNX Spec 2.1 9/3 3.5.3.3.1
 */
enum class BAS_FeatureId
{
    SuppEmiType = 1, //!> Get supported EMI types
    DescrType0  = 2, //!> Get the local Device Descriptor Type 0 for possible local device management
    BusConnStat = 3, //!> Get and inform on the bus connection status
    KnxManCode  = 4, //!> Get the manufacturer code of the Bus Access Server
    ActiveEmi   = 5  //!> Get and set the EMI type to use
};

/**
 * Shall contain the 2 octet KNX manufacturer code<br>
 * We use non existent 0x0815 "manufacturer"
 * @note (KNX Spec 2.1. 9/3 3.5.3.4.1)
 */
#define C_ManufacturerCodeHigh 0x08
#define C_ManufacturerCodeLow  0x15

/**
 * EMI1 message code field (C_MCode_...)
 * @note KNX Spec 2.1 3/6/3
 */
#define C_MCode_TxReq           0x11 //!< L_Data.req / Ein Telegramm von USB auf den KNX-Bus übertragen
#define C_MCode_TxEcho          0x4E //!< L_Data.con / Das Echo vom KNX-IF zurück Richtung USB nach einem TxReq
#define C_MCode_RxData          0x49 //!< L_Data.ind / Ein von KNX empfangenes Telegramm Richtung USB
#define C_MCode_GetValue        0x4C //!< PC_Get_Value.req / Einen Emi-Wert abfragen
#define C_MCode_ResponseValue   0x4B //!< PC_Get_Value.con / Die Antwort auf eine Emi-Wert Abfrage
#define C_MCode_SetValue        0x46 //!< PC_Set_Value.req / Einen Emi-Wert setzen

/**
 * LM_Reset.ind PEI Reset indication
 *
 * @details Should be send only once on hard or soft-reset.
 * @note KNX Spec. 2.1 3/6/2 6.3.2.6 p. 19
 */
#define C_MCode_PEI_Reset       0xA0

// the USB-IF specific masks and commands for the inter-mcu communication
#define C_MCode_USB_IF_SpecialMask   0x80 //!< Selfbus specific !
#define C_MCode_USB_IF_MonitorMask   (~C_MCode_USB_IF_SpecialMask) //!< Selfbus specific !

///\todo Resolve this puzzle.
//#define C_MCode_DbgTx          0xCE //!< Selfbus specific ?
//#define C_MCode_DbgRx          0xC9 //!< Selfbus specific ? Ein von KNX empfangenes Telegramm, nur zum Debug-CDC Interface

#endif /* _KNXUSB_CONST_H */

