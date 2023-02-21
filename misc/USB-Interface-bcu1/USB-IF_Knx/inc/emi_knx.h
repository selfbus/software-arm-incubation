/*
 *  emi_knx.h - Handles the KNX data on the EMI layer
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef EMI_KNX_H_
#define EMI_KNX_H_

#define SYSST_BUSMON 0x90
//    BCU_STATUS_SERIAL_PEI (serial PEI)
#define SYSST_LINKL  0x12 // wird von der ETS genutzt
//    BCU_STATUS_LINK_LAYER (link layer) +
//    BCU_STATUS_SERIAL_PEI (serial PEI)
#define SYSST_TRANSP 0x96
//    BCU_STATUS_LINK_LAYER (link layer) +
//    BCU_STATUS_TRANSPORT_LAYER (transport layer) +
//    BCU_STATUS_SERIAL_PEI (serial PEI)
#define SYSST_APPLL  0x1E // wird von der ETS genutzt
//    BCU_STATUS_LINK_LAYER (link layer) +
//    BCU_STATUS_TRANSPORT_LAYER (transport layer) +
//    BCU_STATUS_APPLICATION_LAYER (application layer) +
//    BCU_STATUS_SERIAL_PEI (serial PEI)
// SYSST_APPLL ist auch der Default- und Reset-Zustand
#define SYSST_RESET  0xC0

class EmiKnxIf
{
public:
  EmiKnxIf(int aLedPin);
  void EmiIf_Tasks(void);
  void RstSysState(void);
  void SetCdcMonMode(bool setreset);
  void SetActivityLed(bool onoff);
  void BlinkActivityLed(void);
  void DoActivityLed(bool LedEnabled);
protected:
  int txbuffno;
  uint8_t firsttxbyte; // muss bloederweise gesichert werden
  uint8_t EmiSystemState;
  bool CdcMonActive;
  bool HidIfActive;
  bool ProcTelWait;
  int LedBlinkCnt;
  int LedTimeCnt;
  int LedPin;
  unsigned LedLastDoTime;
  bool LedEnabled;
  void ReceivedUsbEmiPacket(int buffno);
  uint8_t EmiReadOneVal(int addr);
  void EmiWriteOneVal(int addr, uint8_t value, bool &reset);
  void SetEmiLen(uint8_t *ptr, uint8_t len);
  void SetTPBodyLen(uint8_t *ptr, uint8_t len);
};

extern EmiKnxIf emiknxif;

#endif /* EMI_KNX_H_ */
