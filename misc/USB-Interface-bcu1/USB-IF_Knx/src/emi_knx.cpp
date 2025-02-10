/*
 *  emi_knx.cpp - Handles the KNX data on the EMI layer
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/digital_pin.h>
#include <sblib/eib/knx_lpdu.h>
#include <sblib/eib/userRam.h>
#include <sblib/eib/bus.h>
#include "error_handler.h"
#include "knxusb_const.h"
#include "GenFifo.h"
#include "BufferMgr.h"
#include "emi_knx.h"


#define ACTLED_HPRD 10

EmiKnxIf::EmiKnxIf(BCU1 * bcu,int aLedPin):
    emiBcu(bcu),
    ledPin(aLedPin)
{
    pinMode(ledPin, OUTPUT);
    SetActivityLed(false);
    reset(); // Set bcu in download mode to disable all KNX bus communication
}

void EmiKnxIf::SetActivityLed(bool onoff)
{
    digitalWrite(ledPin, !onoff);
}

void EmiKnxIf::BlinkActivityLed(void)
{
    if (ledEnabled)
    {
        if (ledBlinkCount < 2)
        {
            if (ledBlinkCount == 0)
            {
                SetActivityLed(false);
            }
            ledBlinkCount += 2;
            ledTimeCount = ACTLED_HPRD;
        }
    }
}

void EmiKnxIf::DoActivityLed(bool Led_Enabled)
{
    if (ledEnabled != Led_Enabled)
    {
        ledEnabled = Led_Enabled;
        SetActivityLed(ledEnabled);
    }
    if (ledEnabled)
    {
        if (ledBlinkCount != 0)
        {
            if (ledTimeCount > 0)
              ledTimeCount--;
            if (ledTimeCount == 0)
            {
                ledBlinkCount--;
                if (ledBlinkCount > 0)
                {
                    ledTimeCount = ACTLED_HPRD;
                    SetActivityLed((ledBlinkCount & 1) != 0);
                }
            }
        }
    } else {
        ledBlinkCount = 0;
        ledTimeCount = 0;
    }
}

uint8_t EmiKnxIf::emiReadOneValue(int memoryAddress)
{
    auto isUserRamAddress = emiBcu->userRam->inRange(memoryAddress);
    auto isUserEepromAddress = emiBcu->userEeprom->inRange(memoryAddress);
    if (!isUserRamAddress && !isUserEepromAddress)
    {
        failHardInDebug();
        return 0;
    }

    if (isUserRamAddress)
    {
        switch (memoryAddress)
        {
            case AddrMaskVersionHighByte: // mask version high byte
                return HIGH_BYTE(emiBcu->getMaskVersion());
                break;

            case AddrMaskVersionLowByte: // mask version low byte
                return (lowByte(emiBcu->getMaskVersion()));
                break;

            case AddrSystemState:
              if (getBcuLayerState(BCU_STATUS_PROGRAMMING_MODE))
              {
                  setBcuLayerState(BCU_STATUS_PROGRAMMING_MODE, false);
              }
              return getSystemState();
              break;

            default:
              failHardInDebug();
              return 0;
        }
    }

    if (isUserEepromAddress)
    {
        switch (memoryAddress)
        {
            case AddrIndividualAddressLowByte:
                return lowByte(emiBcu->ownAddress());
                break;
            case AddrIndividualAddressHighByte:
                return HIGH_BYTE(emiBcu->ownAddress());
                break;
            case AddrExpectedPeiType:
                return emiBcu->userEeprom->getUInt8(memoryAddress);
                break;

            case AddrStartAddressTable:
                return emiBcu->userEeprom->getUInt8(memoryAddress);
                break;

            case AddrBaseConfig:
                return emiBcu->userEeprom->getUInt8(memoryAddress);
                break;

            default:
                failHardInDebug();
                return emiBcu->userEeprom->getUInt8(memoryAddress);
        }
    }

    return 0; // we should never land here
}

uint8_t EmiKnxIf::getSystemState()
{
    return emiBcu->userRam->status();
}

void EmiKnxIf::setSystemState(const uint8_t newStatus)
{
    switch(newStatus)
    {
        // "Official" EMI 1 systemStates
        case SystemState::ApplicationLayer:
        case SystemState::BusMonitor:
        case SystemState::LinkLayer:
        case SystemState::TransportLayer:
        case SystemState::Reset:
            emiBcu->userRam->status() = newStatus;
            break;

        // most likely USB-IF specific custom layer changes,
        // e.g. enabling/disabling of CdCMonMode
        default:
            emiBcu->userRam->status() = newStatus;
            break;
    }
}

bool EmiKnxIf::getBcuLayerState(const uint8_t layer)
{
    uint8_t systemState = getSystemState();
    bool layerState = systemState & layer;
    return layerState;
}

void EmiKnxIf::setBcuLayerState(const uint8_t layer, const bool newState)
{
    uint8_t systemState = getSystemState();
    if (newState == (systemState & layer))
    {
        // layer is already in requested state => do nothing;
        return;
    }

    // Set new layer state by flipping layer and parity bit;
    systemState ^= layer | BCU_STATUS_PARITY;
    setSystemState(systemState);
}

void EmiKnxIf::emiWriteOneValue(int addr, uint8_t value, bool &isResetEmi)
{
    if (emiBcu->userRam->inRange(addr) && (!emiBcu->userRam->isStatusAddress(addr)))
    {
        failHardInDebug();
    }

    if (emiBcu->userRam->isStatusAddress(addr))
    {
        isResetEmi = SystemState::Reset == value;
        setSystemState(value);
        return;
    }

    // Vorerst werden die eeprom Schreibzugriffe ungefiltert weitergegeben.
    uint8_t * memoryPtr = emiBcu->userMemoryPtr(addr);
    if (memoryPtr == nullptr)
    {
        failHardInDebug();
        return;
    }

    if (value != emiBcu->userEeprom->getUInt8(addr))
    {
        *emiBcu->userMemoryPtr(addr) = value;
        // bcu.userEeprom->setUInt8(addr, value);
        emiBcu->userEeprom->modified(true);
    }

    if ((addr == AddrIndividualAddressLowByte) || (addr == AddrIndividualAddressHighByte))
    {
        uint16_t newAddress = makeWord(emiBcu->userEeprom->getUInt8(AddrIndividualAddressHighByte),
                                       emiBcu->userEeprom->getUInt8(AddrIndividualAddressLowByte));
        emiBcu->setOwnAddress(newAddress);
    }
    else
    {
        switch (addr)
        {
            case AddrExpectedPeiType:
                break;

            case AddrStartAddressTable:
                break;

            case AddrBaseConfig:
                break;

            default:
                failHardInDebug();
        }
    }
}

void EmiKnxIf::reset(void)
{
    bool dummy;
    emiWriteOneValue(AddrSystemState, SystemState::Reset, dummy);
}

void EmiKnxIf::setTPBodyLength(uint8_t *ptr, uint8_t len)
{
    // Setzt die Telegrammlänge an den verschiedenen Stellen auf die
    // passenden Werte, die Länge ist die des Transfer Protocol Body
                                                                 // len = 9
    // 2 bytes Transfer Protocol Body length
    // KNX Spec 2.1 9/3 3.4.1.3.3
    ptr[2 + C_HRH_HeadLen + IDX_TPH_BodyLen] = HIGH_BYTE(len);       // ptr[7] = 0
    ptr[2 + C_HRH_HeadLen + IDX_TPH_BodyLen + 1] = lowByte(len);     // ptr[8] = len

    // 1 byte HID report frame length
    ptr[2 + IDX_HRH_DataLen] = len + TPH_ProtocolLength_V0;                  // ptr[4] = len + 8

    // 1 byte total length
    ptr[0] = len + TPH_ProtocolLength_V0 + C_HRH_HeadLen + IDX_TPB_Data + 2; // ptr[0] = len + 23
}

void EmiKnxIf::receivedUsbEmiPacket(int buffno)
{
    uint8_t *buffptr = buffmgr.buffptr(buffno);
    // Die ganze Auswertung geht vorerst von EMI1 aus
    // ptr zeigt auf den KNX HID Report Header
    uint8_t *ptr = buffptr + 2 + C_HRH_HeadLen;
    // Jetzt zeigt der ptr auf den KNX HID Report Body
    unsigned TransferBodyLength = (ptr[IDX_TPH_BodyLen] << 8) + ptr[IDX_TPH_BodyLen+1];
    unsigned EmiAddr = (ptr[TPH_ProtocolLength_V0+IDX_TPB_EMI_Addr_h] << 8) + ptr[TPH_ProtocolLength_V0+IDX_TPB_EMI_Addr_l];
    uint8_t len = ptr[TPH_ProtocolLength_V0+IDX_TPB_EMI_Len];
    bool isResetEmi = false;
    uint8_t * ptrTelegramStart;
    switch (ptr[TPH_ProtocolLength_V0]) // Switch auf den EMI M-Code
    {
    case C_MCode_GetValue: // Einen Emi-Wert abfragen
        // Das ankommende Telegramm wird sofort für die Antwort benutzt
        setTPBodyLength(buffptr, len+IDX_TPB_EMI_Data);
        ptr[TPH_ProtocolLength_V0] = C_MCode_ResponseValue;
        ptr += TPH_ProtocolLength_V0+IDX_TPB_EMI_Data;
        while (len > 0)
        {
            *ptr++ = emiReadOneValue(EmiAddr++);
            len--;
        }
        if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
            buffmgr.FreeBuffer(buffno);
        break;

    case C_MCode_SetValue: // Einen Emi-Wert setzen
        ptr += TPH_ProtocolLength_V0+IDX_TPB_EMI_Data;
        while ((len > 0) && !isResetEmi)
        {
            emiWriteOneValue(EmiAddr++, *ptr++, isResetEmi);
            len--;
        }
        if (isResetEmi)
        {
            ///\todo Bug:
            /// after the response is sent, the usb-mcu can't connect a second time e.g. with knxd
            setTPBodyLength(buffptr, 1);
            ptr = buffptr + 2 + C_HRH_HeadLen + TPH_ProtocolLength_V0;
            *ptr++ = C_MCode_PEI_Reset;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
                buffmgr.FreeBuffer(buffno);
        } else {
            buffmgr.FreeBuffer(buffno);
        }
        break;

    case C_MCode_TxReq: // Ein Telegramm von USB auf den KNX-Bus übertragen
        ptrTelegramStart = ptr + TPH_ProtocolLength_V0 + IDX_TPB_Data;
        receivedEmiControlByte = *ptrTelegramStart;
        initLpdu(ptrTelegramStart, priority(ptrTelegramStart), false, FRAME_STANDARD);
        txbuffno = buffno;
        emiBcu->bus->sendTelegram(ptrTelegramStart, TransferBodyLength-1);
        // sendTelegram geht davon aus, dass nach den Telegrammdaten noch 1 Byte frei für die
        // Checksumme ist. Das ist gegeben, die Buffer sind 68 Byte lang für ein 64 Byte HID-Paket.
        // Der Buffer wird erst nach dem Versenden wieder freigegeben
        BlinkActivityLed();
        break;

    default:
        buffmgr.FreeBuffer(buffno);
  }
}

void EmiKnxIf::sendReceivedTelegramAsEMI(uint8_t * telegram, uint8_t length)
{
    int buffno = buffmgr.AllocBuffer();
    if (buffno < 0)
    {
        failHardInDebug();
        return;
    }

    uint8_t *buffptr = buffmgr.buffptr(buffno);
    setTPBodyLength(buffptr, length); // [0], [4], [7,8] length positions
    buffptr += 2; // skip [0] (total length) and [1] (checksum) (already set in setTPBodyLength(.))

    // create HID report header (HRH)
    *buffptr++ = HRH_ReportID; // [2] HRH report id
    *buffptr++ = C_HRH_PacketInfoSinglePacket; // [3] HRH packet info
    buffptr++; // skip [4] HID report frame length (already set in setTPBodyLength(.))

    // create KNX USB Transfer Protocol Header of HID report body (HRB)
    *buffptr++ = TPH_ProtocolVersion_V0; // [5]
    *buffptr++ = TPH_ProtocolLength_V0;  // [6]
    buffptr += 2; // skip [7] & [8] Transfer Protocol Body length (already set in setTPBodyLength(.))
    *buffptr++ = TPH_ProtocolID::knxTunnel; // [9]
    *buffptr++ = TPH_EMI_ID::EMI1; // [10]
    *buffptr++ = TPH_ManufacturerCode_V0_HIGH_BYTE; // [11]
    *buffptr++ = TPH_ManufacturerCode_V0_LOW_BYTE; //  [12]

    // create KNX USB Transfer Protocol Body (TPB)
    // set EMI message code
    uint8_t emiMessageCode;
    if (deviceMode == DeviceMode::HidOnly)
    {
        // Standard EMI message
        emiMessageCode = C_MCode_RxData;
    }
    else
    {
        // "Special" EMI message for inter-mcu communication
        emiMessageCode = (C_MCode_RxData | C_MCode_USB_IF_SpecialMask);
    }
    *buffptr++ = emiMessageCode; // [13]

    // set EMI data (KNX frame)
    for (int i = 0; i < length; ++i) // [14 - x]
    {
        *buffptr++ = telegram[i];
    }

    // push buffer to serial
    if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
    {
        buffmgr.FreeBuffer(buffno);
    }

    BlinkActivityLed();
}

// Die bcu.bus->handleTelegram() händelt die Layer 2 LL_ACKS (1 Byte) intern.
// Alle "interessanten" Telegramme sind >= 8 Bytes lang. Über USB werden sie ohne Checksumme getunnelt,
// dort sind sie also >= 7 Bytes lang.
// Es ist möglich, alle Telegramme >= 8 Bytes an einem Monitor vorbeizuschleusen, indem:
// In bcu.userRam->status() das Bit BCU_STATUS_TRANSPORT_LAYER gelöscht wird. Dann ruft BcuBase.loop() nie processTelegram()
// auf und die Telegramme sind noch vorhanden, wenn loop() der Applikation aufgerufen wird.
// Dort kann dann das Telegramm kopiert und danach bei Bedarf processTelegramm() aufgerufen werden. Muss das
// überhaupt? Es wäre praktisch, wenn man über den Bus die Adresse schreiben kann, insofern also doch?
// Wenn Bit BCU_STATUS_TRANSPORT_LAYER gelöscht wird, MUSS auch BCU_STATUS_LINK_LAYER gelöscht werden,
// ansonsten wird jedes Telegram von uns mit einem LL_ACK beantwortet.

// Also: CdcMonActive ist wahr: Dann werden die Telegramme hier vorselektiert und der sblib übergeben
// CdcActive wird deaktiviert (und isHidActive() ist false): Jetzt muss die Kontrolle wieder zurück
// rein an die sblib übertragen werden. Das letzt empfangene und noch im Buffer stehende Telegramm könnte
// noch ein ungefiltertes sein, dass muss also dennoch hier vorsortiert und evtl an die Sblib übergeben
// werden.
void EmiKnxIf::EmiIf_Tasks(void)
{
    if (emiBcu->bus->sendingFrame())
    {
        return;
    }

    // Check and handle received KNX bus telegram
    if (emiBcu->bus->telegramReceived())
    {
        BlinkActivityLed();
        if (isKNXenabled() && (getBcuLayerState(BCU_STATUS_SERIAL_PEI)))
        {
            sendReceivedTelegramAsEMI(emiBcu->bus->telegram, emiBcu->bus->telegramLen);
        }
        else
        {
            handleTelegramForUs(emiBcu->bus->telegram, emiBcu->bus->telegramLen);
        }
        emiBcu->bus->discardReceivedTelegram();
    }

    // Das Response-Telegramm Richtung USB schicken.
    if (txbuffno >= 0)
    {
        // Dafür kann der Buffer wiederverwendet werden,
        // denn in ihm steht das vollständige HID-Paket vom Hinweg.
        uint8_t *ptr = buffmgr.buffptr(txbuffno);
        // Emi-Typ ändern
        ptr[2+C_HRH_HeadLen+TPH_ProtocolLength_V0] = C_MCode_TxEcho;
        // SendTelegram hat die lokale Adresse bereits hinzugefügt
        // Jetzt muss noch das erste Byte des Telegramms rekonstruiert werden
        ptr[2+C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_Data] = receivedEmiControlByte;
        // Zum Verschicken einreihen
        if (ser_txfifo.Push(txbuffno) != TFifoErr::Ok)
        {
            buffmgr.FreeBuffer(txbuffno);
        }
        txbuffno = -1;
    }

    // Check and handle received EMI (serial) telegram
    if ((txbuffno == -1) && (hid_txfifo.Empty() != TFifoErr::Empty))
    {
        int buffno;
        hid_txfifo.Pop(buffno);
        uint8_t *ptr = buffmgr.buffptr(buffno);
        unsigned UartPacketLength = ptr[0];
        unsigned HidPacketLength = ptr[2+IDX_HRH_DataLen]+3;
        // Der Check ist eigentlich nicht notwendig, ist nur eine kurze Plausibilisierung.
        // Ausführlicher wurde schon auf der USB-Seite geprüft.
        if ((ptr[2+IDX_HRH_Id] == C_HRH_IdHid) && (UartPacketLength == (HidPacketLength+2)))
        {
            receivedUsbEmiPacket(buffno);
        }
        else
        {
            buffmgr.FreeBuffer(buffno);
        }
    }

    // handle activity led
    if (elapsed(ledLastDoTime) >= 10)
    {
        ledLastDoTime = millis();
        DoActivityLed(isKNXenabled());
    }
}

void EmiKnxIf::handleTelegramForUs(uint8_t * telegram, uint8_t lenght)
{
    uint16_t destAddr = makeWord(telegram[3], telegram[4]);
    bool processTel = false;

    // Only process the telegram if it is for us
    if (telegram[5] & 0x80) // group address or physical address
    {
        processTel = (destAddr == 0); // broadcast
        processTel |= (emiBcu->addrTables != nullptr) && (emiBcu->addrTables->indexOfAddr(destAddr) >= 0); // known group address
    }
    else if (destAddr == emiBcu->ownAddress())
    {
        processTel = true;
    }

    if (processTel)
    {
        emiBcu->processTelegram(&telegram[0], lenght);
    }
}

void EmiKnxIf::setDeviceMode(DeviceMode newDeviceMode)
{
    bool dummy;
    deviceMode = newDeviceMode;
    switch (deviceMode)
    {
        case DeviceMode::Invalid:
        case DeviceMode::Halt:
        case DeviceMode::ProgBusChip:
        case DeviceMode::ProgUserChip:
        case DeviceMode::HidOnly: // Reset also on HidOnly and wait till ETS/knxd/calimero
                                  // sets the correct SystemState via C_MCode_SetValue with address AddrSystemState
            reset();
            break;

        case DeviceMode::UsbMon:
            emiWriteOneValue(AddrSystemState, SystemState::LinkLayer, dummy); // sets BCU_STATUS_SERIAL_PEI, BCU_STATUS_LINK_LAYER
            break;

        case DeviceMode::BusMon:
            emiWriteOneValue(AddrSystemState, SystemState::BusMonitor, dummy); // sets BCU_STATUS_SERIAL_PEI
            break;

        default:
            // This should never happen.
            // If you land here, check that the switch statement checks all DeviceMode
            failHardInDebug();
            break;
    }
}

bool EmiKnxIf::isKNXenabled()
{
    bool result = ((deviceMode == DeviceMode::HidOnly) ||
                   (deviceMode == DeviceMode::UsbMon) ||
                   (deviceMode == DeviceMode::BusMon));
    return result;
}
