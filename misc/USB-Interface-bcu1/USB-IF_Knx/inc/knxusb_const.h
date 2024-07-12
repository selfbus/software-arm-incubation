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
// Die Adressen sind ab Anfang HID Report Header
#define A_HRH_Id      0
#define C_HRH_IdHid   1
#define C_HRH_IdCdc   2
#define C_HRH_IdDev   3
//#define A_HRH_PkInfo  1
/**
 * Single Packet (value 0x13), mehr erst mal nicht unterstuetzen
 */
#define C_HRH_PacketInfoSinglePacket (HRH_SequenceNumber | HRH_PacketType::startPacket | HRH_PacketType::endPacket)
#define A_HRH_DataLen 2
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
constexpr uint16_t TPH_ManufacturerCode_V0 = 0x0000;


// Transfer Protocol Header - TPH
// Die Adressen sind ab Anfang Transfer Protocol Header
//#define A_TPH_Version 0
//#define C_TPH_Version 0
//#define A_TPH_HeadLen 1
#define C_TPH_HeadLen 8
#define A_TPH_BodyLen 2
//#define A_TPH_ProtId  4
//#define C_TPH_PId_KnxTunnel 1
//#define C_TPH_PId_BAS    0x0f
//#define A_TPH_EmiId      5
//#define C_TPH_EmiId_Emi1 1
//#define C_TPH_EmiId_Emi2 2
//#define C_TPH_EmiId_cEmi 3
//#define A_TPH_SerId      5
////#define C_TPH_SerId 1
//#define A_TPH_ManuCode1 6
//#define A_TPH_ManuCode2 7
//#define C_TPH_ManuCode1 0
//#define C_TPH_ManuCode2 0

// Transfer Protocol Body - TPB
//#define A_TPB_FeatureId   0
//#define A_TPB_FeatureData 1
//#define A_TPB_MCode       0
#define A_TPB_Data        1 //!< wenn KNX-Frame
#define A_TPB_EMI_Len     1 //!< wenn EMI service
#define A_TPB_EMI_Addr_h  2
#define A_TPB_EMI_Addr_l  3
#define A_TPB_EMI_Data    4

// Bus Access Server - BAS
//#define BAS_ServiceId_FeatureGet  1
//#define BAS_ServiceId_FeatureResp 2
//#define BAS_ServiceId_FeatureSet  3
//#define BAS_ServiceId_FeatureInfo 4
//#define BAS_FeatureId_SuppEmiType 1
//#define BAS_FeatureId_DescrType0  2
//#define BAS_FeatureId_BusConnStat 3
//#define BAS_FeatureId_KnxManCode  4
//#define BAS_FeatureId_ActiveEmi   5

//#define C_ManufacturerCodeHigh 0x08
//#define C_ManufacturerCodeLow  0x15

/**
 * EMI1 message code field (C_MCode_...)
 * @note KNX Spec 2.1 3/6/3
 */
//#define C_MCode_DbgTx          0xCE //!< Selfbus specific ?
//#define C_MCode_DbgRx          0xC9 //!< Selfbus specific ? Ein von KNX empfangenes Telegramm, nur zum Debug-CDC Interface
#define C_MCode_TxReq           0x11 //!< L_Data.req / Ein Telegramm von USB auf den KNX-Bus übertragen
#define C_MCode_TxEcho          0x4E //!< L_Data.con / Das Echo vom KNX-IF zurück Richtung USB nach einem TxReq
#define C_MCode_RxData          0x49 //!< L_Data.ind / Ein von KNX empfangenes Telegramm Richtung USB
#define C_MCode_GetValue        0x4C //!< PC_Get_Value.req / Einen Emi-Wert abfragen
#define C_MCode_ResponseValue   0x4B //!< PC_Get_Value.con / Die Antwort auf eine Emi-Wert Abfrage
#define C_MCode_SetValue        0x46 //!< PC_Set_Value.req / Einen Emi-Wert setzen
#define C_MCode_ResetResponse   0xA0
//#define C_MCode_MonMask        0x7f //!< Selfbus specific ?
#define C_MCode_SpecMsk         0x80 //!< Selfbus specific ?

#endif /* _KNXUSB_CONST_H */

