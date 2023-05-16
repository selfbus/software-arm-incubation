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
#include <sblib/eib/userRam.h>
#include <sblib/eib/bus.h>
#include <sblib/eib/bcu1.h>
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
  HidIfActive = false;
  ProcTelWait = false;
  EmiSystemState = SYSST_APPLL;
  LedEnabled = true; // Damit die LED beim Start AUSgeschaltet wird
  LedPin = aLedPin;
  pinMode(LedPin, OUTPUT);
  SetActivityLed(false);
  LedLastDoTime = 0;
}

void EmiKnxIf::SetActivityLed(bool onoff)
{
  digitalWrite(LedPin, !onoff);
}

void EmiKnxIf::BlinkActivityLed(void)
{
  if (LedEnabled)
  {
    if (LedBlinkCnt < 2)
    {
      if (LedBlinkCnt == 0)
      {
        SetActivityLed(false);
      }
      LedBlinkCnt += 2;
      LedTimeCnt = ACTLED_HPRD;
    }
  }
}

void EmiKnxIf::DoActivityLed(bool Led_Enabled)
{
  if (LedEnabled != Led_Enabled)
  {
    LedEnabled = Led_Enabled;
    SetActivityLed(LedEnabled);
  }
  if (LedEnabled)
  {
    if (LedBlinkCnt != 0)
    {
      if (LedTimeCnt > 0)
        LedTimeCnt--;
      if (LedTimeCnt == 0)
      {
        LedBlinkCnt--;
        if (LedBlinkCnt > 0)
        {
          LedTimeCnt = ACTLED_HPRD;
          SetActivityLed((LedBlinkCnt & 1) != 0);
        }
      }
    }
  } else {
    LedBlinkCnt = 0;
    LedTimeCnt = 0;
  }
}

uint8_t EmiKnxIf::EmiReadOneVal(int addr)
{
  if (addr < 0x100)
  {
    if (addr == 0x4E)
      return 0; // Mask Type 00=TP-BCU
    if (addr == 0x4F)
      return (bcu.getMaskVersion()); ///\todo why only low byte returning?
    if (addr == 0x60)
    {
      //if (userRam.status() & BCU_STATUS_PROGRAMMING_MODE)
      //  return EmiSystemState ^ (BCU_STATUS_PROGRAMMING_MODE | BCU_STATUS_PARITY);
      //else
        return EmiSystemState;
    }
    return 0;
  }
  if (addr < 0x200)
  {
//    if (addr == 0x117)
//      return bus.ownAddress() & 0xff;
//    if (addr == 0x118)
//      return bus.ownAddress() >> 8;
    return bcu.userEeprom->getUInt8(addr);
  }
  return 0;
}

void EmiKnxIf::SetCdcMonMode(bool setreset)
{
  CdcMonActive = setreset;
}

/*
 * Virtueller EmiSystemState
 * Wenn der BusMonitor-Modus aktiv ist, dann ist das If selbst immer im Monitor-Modus.
 * Gegenüber dem Hid-If muss das jedoch gefiltert werden.
 * Wenn das Hid-If den Monitor-Modus verlangt, muss das natürlich entsprechend umgesetzt werden.
 */

void EmiKnxIf::EmiWriteOneVal(int addr, uint8_t value, bool &reset)
{
  reset = false;
  if (addr == 0x60)
  {
    // EmiSystemState setzen
    switch (value)
    {
    case SYSST_BUSMON: // Busmonitor ETS sends this with bus monitor start
      //userRam.status() = value; //
      EmiSystemState = SYSST_BUSMON; ///\todo ETS busmonitor mode still doesnt work
      HidIfActive = true;
      break;
    case SYSST_LINKL:  // LinkLayer, in diesem Modus kommuniziert die ETS mit dem Bus
      //userRam.status() = value;
      EmiSystemState = SYSST_LINKL;
      HidIfActive = true;
      break;
    case SYSST_TRANSP:
      //userRam.status() = value; // Bisher nie beobachtet
      break;
    case SYSST_APPLL: // Das ist der Reset-Default, keine Übertragung zum Hid-If
      //userRam.status() = value;
      EmiSystemState = SYSST_APPLL;
      HidIfActive = false;
      break;
    case SYSST_RESET:
      // Reset, und nu?
      //userRam.status() = SYSST_APPLL;
      EmiSystemState = SYSST_APPLL;
      HidIfActive = false;
      reset = true;
    default: ;
    }
  }
  // Vorerst werden die Schreibzugriffe ungefiltert weitergegeben.
  if ((addr >= 0x100) && (addr < 0x200))
  {
      *bcu.userMemoryPtr(addr) = value;
  }
}

void EmiKnxIf::RstSysState(void)
{
  bool rst;
  EmiWriteOneVal(0x60, SYSST_RESET, rst);
}

void EmiKnxIf::SetTPBodyLen(uint8_t *ptr, uint8_t len)
{
  // Setzt die Telegrammlänge an den verschiedenen Stellen auf die
  // passenden Werte, die Länge ist die des Transfer Protocol Body
  ptr[2 + C_HRH_HeadLen + A_TPH_BodyLen+1] = len;
  ptr[2 + A_HRH_DataLen] = len+C_TPH_HeadLen;
  ptr[0] = len+C_TPH_HeadLen+C_HRH_HeadLen+A_TPB_Data+2;
}

void EmiKnxIf::ReceivedUsbEmiPacket(int buffno)
{
  uint8_t *buffptr = buffmgr.buffptr(buffno);
  // Die ganze Auswertung geht vorerst von EMI1 aus
  // ptr zeigt auf den KNX HID Report Header
  uint8_t *ptr = buffptr + 2 + C_HRH_HeadLen;
  // Jetzt zeigt der ptr auf den KNX HID Report Body
  unsigned TransferBodyLength = (ptr[A_TPH_BodyLen] << 8) + ptr[A_TPH_BodyLen+1];
  unsigned EmiAddr = (ptr[C_TPH_HeadLen+A_TPB_EMI_Addr_h] << 8) + ptr[C_TPH_HeadLen+A_TPB_EMI_Addr_l];
  uint8_t len = ptr[C_TPH_HeadLen+A_TPB_EMI_Len];
  bool reset = false;
  switch (ptr[C_TPH_HeadLen]) // Switch auf den EMI M-Code
  {
  case C_MCode_GetValue: // Einen Emi-Wert abfragen
    // Das ankommende Telegramm wird sofort für die Antwort benutzt
    SetTPBodyLen(buffptr, len+A_TPB_EMI_Data);
    ptr[C_TPH_HeadLen] = C_MCode_RespValue;
    ptr += C_TPH_HeadLen+A_TPB_EMI_Data;
    while (len > 0)
    {
      *ptr++ = EmiReadOneVal(EmiAddr++);
      len--;
    }
    if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
      buffmgr.FreeBuffer(buffno);
    break;
  case C_MCode_SetValue: // Einen Emi-Wert setzen
    ptr += C_TPH_HeadLen+A_TPB_EMI_Data;
    while ((len > 0) && !reset)
    {
      EmiWriteOneVal(EmiAddr++, *ptr++, reset);
      len--;
    }
    if (reset)
    {
      SetTPBodyLen(buffptr, 1);
      ptr = buffptr + 2 + C_HRH_HeadLen + C_TPH_HeadLen;
      *ptr++ = C_MCode_RstResp;
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
    firsttxbyte = ptr[C_TPH_HeadLen+A_TPB_Data];
    txbuffno = buffno;
    bcu.bus->sendTelegram(ptr+C_TPH_HeadLen+A_TPB_Data, TransferBodyLength-1);
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
 * CdcActive wird deaktiviert (und HidIfActive ist false): Jetzt muss die Kontrolle wieder zurück
 * rein an die sblib übertragen werden. Das letzt empfangene und noch im Buffer stehende Telegramm könnte
 * noch ein ungefiltertes sein, dass muss also dennoch hier vorsortiert und evtl an die Sblib übergeben
 * werden.
 */
void EmiKnxIf::EmiIf_Tasks(void)
{
  bool KnxProcActive;
  bool lastinternal = false;
  // userRam.status() aktualisieren
  if (CdcMonActive || HidIfActive)
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
    if (HidIfActive)
      KnxProcActive = false;
    else
      KnxProcActive = true; // Hier in EmiIf_Tasks muss die Tel.abarbeitung der sblib aufgerufen werden
  else
    KnxProcActive = false;

  if (bcu.bus->telegramReceived())
  {
    if (!ProcTelWait && (CdcMonActive || HidIfActive))
    {
      int buffno = buffmgr.AllocBuffer();
      if (buffno >= 0)
      {
        uint8_t *buffptr = buffmgr.buffptr(buffno);
        SetTPBodyLen(buffptr, bcu.bus->telegramLen);
        buffptr += 2;
        *buffptr++ = 0x01;
        *buffptr++ = 0x13;
        buffptr++;
        *buffptr++ = 0;
        *buffptr++ = 0x08;
        *buffptr++ = 0;
        buffptr++;
        *buffptr++ = 0x01;
        *buffptr++ = 0x01;
        *buffptr++ = 0;
        *buffptr++ = 0;
        *buffptr++ = (HidIfActive) ? C_MCode_RxData:(C_MCode_RxData|C_MCode_SpecMsk);
        for (int i = 0; i < bcu.bus->telegramLen; ++i)
          *buffptr++ = bcu.bus->telegram[i];
        if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
          buffmgr.FreeBuffer(buffno);
      }
      BlinkActivityLed();
    }
    // Wenn KnxProcActive dann muss hier die Bearbeitung der Telegramme wie sonst in der sblib angestoßen werden.
    if (KnxProcActive || lastinternal)
    {
      bool processTel = false;
      ProcTelWait = false;

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
          while (bcu.bus->sendingTelegram())
          { // Warte max 100 ms
            if ((millis() - starttime) > 100)
            {
              lastinternal = false;
              ProcTelWait = false;
              bcu.bus->discardReceivedTelegram();
            }
          }
        }
        if (bcu.bus->telegramReceived())
        {
          if (!bcu.bus->sendingTelegram())
          {
            bcu.processTelegram(&bcu.bus->telegram[0], bcu.bus->telegramLen);
          } else {
            ProcTelWait = true;
          }
        }
      } else {
        ProcTelWait = false;
        bcu.bus->discardReceivedTelegram();
      }
    } else {
      ProcTelWait = false;
      bcu.bus->discardReceivedTelegram();
    }
  }

  if ((txbuffno >= 0) && (!bcu.bus->sendingTelegram()) && !ProcTelWait)
  {
    // Das Response-Telegramm Richtung USB schicken. Dafür kann der Buffer
    // wiederverwendet werden, denn in ihm steht das vollständige HID-Paket
    // vom Hinweg.
    uint8_t *ptr = buffmgr.buffptr(txbuffno);
    // Emi-Typ ändern
    ptr[2+C_HRH_HeadLen+C_TPH_HeadLen] = C_MCode_TxEcho;
    // SendTelegram hat die lokale Adresse bereits hinzugefügt
    // Jetzt muss noch das erste Byte des Telegramms rekonstruiert werden
    ptr[2+C_HRH_HeadLen+C_TPH_HeadLen+A_TPB_Data] = firsttxbyte;
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
    unsigned HidPacketLength = ptr[2+A_HRH_DataLen]+3;
    // Der Check ist eigentlich nicht notwendig, ist nur eine kurze Plausibilisierung.
    // Ausführlicher wurde schon auf der USB-Seite geprüft.
    if ((ptr[2+A_HRH_Id] == C_HRH_IdHid) && (UartPacketLength == (HidPacketLength+2)))
      ReceivedUsbEmiPacket(buffno);
    else
      buffmgr.FreeBuffer(buffno);
  }

  if ((millis() - LedLastDoTime) >= 10)
  {
    LedLastDoTime = millis();
    DoActivityLed(CdcMonActive || HidIfActive);
  }
}
