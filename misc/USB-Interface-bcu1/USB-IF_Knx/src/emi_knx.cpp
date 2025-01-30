/*
 *  emi_knx.cpp - Handles the KNX data on the EMI layer
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include <sblib/digital_pin.h>
#include <sblib/eib/knx_lpdu.h>
#include <sblib/eib/userRam.h>
#include <sblib/eib/bus.h>
#include <sblib/eib/bcu1.h>
#include "error_handler.h"
#include "knxusb_const.h"
#include "GenFifo.h"
#include "BufferMgr.h"
#include "emi_knx.h"

EmiKnxIf emiknxif(PIO1_5);

extern BCU1 bcu;

#define ACTLED_HPRD 10

EmiKnxIf::EmiKnxIf(int aLedPin)
{
  txbuffno = -1;
  CdcMonActive = false;
  hidIfActive = false;
  procTelWait = false;
  EmiSystemState = SYSST_APPLL;
  ledEnabled = true; // Damit die LED beim Start AUSgeschaltet wird
  ledPin = aLedPin;
  pinMode(ledPin, OUTPUT);
  SetActivityLed(false);
  ledLastDoTime = 0;
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
    auto isUserRamAddress = bcu.userRam->inRange(memoryAddress);
    auto isUserEepromAddress = bcu.userEeprom->inRange(memoryAddress);
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
                return HIGH_BYTE(bcu.getMaskVersion());
                break;

            case AddrMaskVersionLowByte: // mask version low byte
                return (lowByte(bcu.getMaskVersion()));
                break;

            case AddrSystemState:
              //if (userRam.status() & BCU_STATUS_PROGRAMMING_MODE)
              //  return EmiSystemState ^ (BCU_STATUS_PROGRAMMING_MODE | BCU_STATUS_PARITY);
              //else
              return EmiSystemState;
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
                return lowByte(bcu.ownAddress());
                break;
            case AddrIndividualAddressHighByte:
                return HIGH_BYTE(bcu.ownAddress());
                break;
            case AddrExpectedPeiType:
                return bcu.userEeprom->getUInt8(memoryAddress);
                break;

            case AddrStartAddressTable:
                return bcu.userEeprom->getUInt8(memoryAddress);
                break;

            case AddrBaseConfig:
                return bcu.userEeprom->getUInt8(memoryAddress);
                break;

            default:
                failHardInDebug();
                return bcu.userEeprom->getUInt8(memoryAddress);
        }
    }

    return 0; // we should never land here
}

void EmiKnxIf::SetCdcMonMode(bool setreset)
{
  CdcMonActive = setreset;
}

void EmiKnxIf::setEmiSystemState(uint8_t newValue, bool &reset)
{
    reset = false;
    switch (newValue)
    {
        case SYSST_BUSMON: // Busmonitor ETS sends this with bus monitor start
            //userRam.status() = value; //
            EmiSystemState = SYSST_BUSMON; ///\todo ETS busmonitor mode still doesnt work
            hidIfActive = true;
            break;

        case SYSST_LINKL:  // LinkLayer, in diesem Modus kommuniziert die ETS mit dem Bus
            //userRam.status() = value;
            EmiSystemState = SYSST_LINKL;
            hidIfActive = true;
            break;

        case SYSST_TRANSP:
            //userRam.status() = value; // Bisher nie beobachtet
            failHardInDebug();
            break;

        case SYSST_APPLL: // Das ist der Reset-Default, keine Übertragung zum Hid-If
            //userRam.status() = value;
            EmiSystemState = SYSST_APPLL;
            hidIfActive = false;
            break;

        case SYSST_RESET:
            // Reset, und nu?
            //userRam.status() = SYSST_APPLL;
            EmiSystemState = SYSST_APPLL;
            hidIfActive = false;
            reset = true;
            break;

        default:
            failHardInDebug();
    }
}

void EmiKnxIf::emiWriteOneValue(int addr, uint8_t value, bool &reset)
{
  if (bcu.userRam->inRange(addr) && (!bcu.userRam->isStatusAddress(addr)))
  {
      failHardInDebug();
  }

  if (bcu.userRam->isStatusAddress(addr))
  {
    setEmiSystemState(value, reset);
    return;
  }

  // Vorerst werden die eeprom Schreibzugriffe ungefiltert weitergegeben.
  uint8_t * memoryPtr = bcu.userMemoryPtr(addr);
  if (memoryPtr == nullptr)
  {
      failHardInDebug();
      return;
  }

  if (value != bcu.userEeprom->getUInt8(addr))
  {
      *bcu.userMemoryPtr(addr) = value;
      // bcu.userEeprom->setUInt8(addr, value);
      bcu.userEeprom->modified(true);
  }

  if ((addr == AddrIndividualAddressLowByte) || (addr == AddrIndividualAddressHighByte))
  {
      uint16_t newAddress = makeWord(bcu.userEeprom->getUInt8(AddrIndividualAddressHighByte),
                                     bcu.userEeprom->getUInt8(AddrIndividualAddressLowByte));
      bcu.setOwnAddress(newAddress);
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

void EmiKnxIf::resetSystemState(void)
{
  bool rst;
  emiWriteOneValue(AddrSystemState, SYSST_RESET, rst);
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
  bool reset = false;
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
    while ((len > 0) && !reset)
    {
      emiWriteOneValue(EmiAddr++, *ptr++, reset);
      len--;
    }
    if (reset)
    {
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
    bcu.bus->sendTelegram(ptrTelegramStart, TransferBodyLength-1);
    // sendTelegram geht davon aus, dass nach den Telegrammdaten noch 1 Byte frei für die
    // Checksumme ist. Das ist gegeben, die Buffer sind 68 Byte lang für ein 64 Byte HID-Paket.
    // Der Buffer wird erst nach dem Versenden wieder freigegeben
    BlinkActivityLed();
    break;
  default:
    buffmgr.FreeBuffer(buffno);
  }
}

/*
 * Also: CdcMonActive ist wahr: Dann werden die Telegramme hier vorselektiert und der sblib übergeben
 * CdcActive wird deaktiviert (und hidIfActive ist false): Jetzt muss die Kontrolle wieder zurück
 * rein an die sblib übertragen werden. Das letzt empfangene und noch im Buffer stehende Telegramm könnte
 * noch ein ungefiltertes sein, dass muss also dennoch hier vorsortiert und evtl an die Sblib übergeben
 * werden.
 */
void EmiKnxIf::EmiIf_Tasks(void)
{
  bool KnxProcActive;
  bool lastinternal = false;
  // userRam.status() aktualisieren
  if (CdcMonActive || hidIfActive)
  {
    if (bcu.userRam->status() & BCU_STATUS_TRANSPORT_LAYER)
        bcu.userRam->status() ^= BCU_STATUS_TRANSPORT_LAYER | BCU_STATUS_PARITY; // interne TL Verarbeitung deaktivieren
    if (bcu.userRam->status() & BCU_STATUS_LINK_LAYER)
        bcu.userRam->status() ^= BCU_STATUS_LINK_LAYER | BCU_STATUS_PARITY; // interne LL Verarbeitung deaktivieren
  } else {
    if ((bcu.userRam->status() & BCU_STATUS_TRANSPORT_LAYER) == 0)
    {
        bcu.userRam->status() ^= BCU_STATUS_TRANSPORT_LAYER | BCU_STATUS_PARITY; // interne TL Verarbeitung aktivieren
      lastinternal = true;
    }
    if ((bcu.userRam->status() & BCU_STATUS_LINK_LAYER) == 0)
    {
        bcu.userRam->status() ^= BCU_STATUS_LINK_LAYER | BCU_STATUS_PARITY; // interne LL Verarbeitung aktivieren
    }

  }

  if (CdcMonActive)
    if (hidIfActive)
      KnxProcActive = false;
    else
      KnxProcActive = true; // Hier in EmiIf_Tasks muss die Tel.abarbeitung der sblib aufgerufen werden
  else
    KnxProcActive = false;

  //KnxProcActive = CdcMonActive && !hidIfActive;
  if (KnxProcActive != CdcMonActive && !hidIfActive) ///\todo should simplify above 2 ifs
      failHardInDebug();

  if (bcu.bus->telegramReceived())
  {
    if (!procTelWait && (CdcMonActive || hidIfActive))
    {
      int buffno = buffmgr.AllocBuffer();
      if (buffno >= 0)
      {
        uint8_t *buffptr = buffmgr.buffptr(buffno);
        setTPBodyLength(buffptr, bcu.bus->telegramLen); // [0], [4], [7,8] length positions
        ///\todo missing position 1 ?
        buffptr += 2; // skip [0] (total length) and [1] (unknown usage) (already set in setTPBodyLength(.))

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
        if (hidIfActive)
            emiMessageCode = C_MCode_RxData;
        else
            emiMessageCode = (C_MCode_RxData | C_MCode_USB_IF_SpecialMask);
        *buffptr++ = emiMessageCode; // [13]

        // set EMI data (KNX frame)
        for (int i = 0; i < bcu.bus->telegramLen; ++i) // [14 - x]
          *buffptr++ = bcu.bus->telegram[i];

        // push buffer to serial
        if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
          buffmgr.FreeBuffer(buffno);
      }
      BlinkActivityLed();
    }
    // Wenn KnxProcActive dann muss hier die Bearbeitung der Telegramme wie sonst in der sblib angestoßen werden.
    if (KnxProcActive || lastinternal)
    {
      bool processTel = false;
      procTelWait = false;

      // Nur weiter verarbeiten, wenn es an uns gerichtet ist
      int destAddr = (bcu.bus->telegram[3] << 8) | bcu.bus->telegram[4];
      if (bcu.bus->telegram[5] & 0x80)
      {
          if ((destAddr == 0) || (bcu.addrTables->indexOfAddr(destAddr) >= 0))
              processTel = true;
      }
      else if (destAddr == bcu.ownAddress())
      {
        processTel = true;
      }

      if (processTel)
      {
        if (lastinternal)
        { // Dann kann es nicht auf den nächsten Schleifendurchlauf verschoben werden
          unsigned int starttime = millis();
          while (bcu.bus->sendingFrame())
          { // Warte max 100 ms
            if ((millis() - starttime) > 100)
            {
              lastinternal = false;
              procTelWait = false;
              bcu.bus->discardReceivedTelegram();
            }
          }
        }
        if (bcu.bus->telegramReceived())
        {
          if (!bcu.bus->sendingFrame())
          {
            bcu.processTelegram(&bcu.bus->telegram[0], bcu.bus->telegramLen);
          } else {
            procTelWait = true;
          }
        }
      } else {
        procTelWait = false;
        bcu.bus->discardReceivedTelegram();
      }
    } else {
      procTelWait = false;
      bcu.bus->discardReceivedTelegram();
    }
  }

  if ((txbuffno >= 0) && (!bcu.bus->sendingFrame()) && !procTelWait)
  {
    // Das Response-Telegramm Richtung USB schicken. Dafür kann der Buffer
    // wiederverwendet werden, denn in ihm steht das vollständige HID-Paket
    // vom Hinweg.
    uint8_t *ptr = buffmgr.buffptr(txbuffno);
    // Emi-Typ ändern
    ptr[2+C_HRH_HeadLen+TPH_ProtocolLength_V0] = C_MCode_TxEcho;
    // SendTelegram hat die lokale Adresse bereits hinzugefügt
    // Jetzt muss noch das erste Byte des Telegramms rekonstruiert werden
    ptr[2+C_HRH_HeadLen+TPH_ProtocolLength_V0+IDX_TPB_Data] = receivedEmiControlByte;
    // Zum Verschicken einreihen
    if (ser_txfifo.Push(txbuffno) != TFifoErr::Ok)
      buffmgr.FreeBuffer(txbuffno);
    txbuffno = -1;
  }

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
      receivedUsbEmiPacket(buffno);
    else
      buffmgr.FreeBuffer(buffno);
  }

  if (elapsed(ledLastDoTime) >= 10)
  {
    ledLastDoTime = millis();
    DoActivityLed(CdcMonActive || hidIfActive);
  }
}
