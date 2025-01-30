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

constexpr uint16_t AddrSystemState         = 0x60; ///\todo replace with bcu.userRam->statusOffset() // system state
constexpr uint16_t AddrMaskVersionHighByte = 0x4E; //!< MaskVersion high byte
constexpr uint16_t AddrMaskVersionLowByte  = 0x4F; //!< MaskVersion low byte

/**
 * From tuwien.auto.calimero.link.BcuSwitcher:<br>
 * Ext Busmon mode to obtain 2 byte domain address: switch at address 0x0101 bit 3 to 0<br>
 * Domain address shall be in range [0x01 .. 0xFF], 0x0 is system broadcast
 */
constexpr uint16_t AddrBaseConfig = 0x101;
constexpr uint16_t AddrExpectedPeiType = 0x109;            //!< PEI type that the application program requires
constexpr uint16_t AddrStartAddressTable = 0x116;          //!< Start of address table
constexpr uint16_t AddrIndividualAddressHighByte = 0x117;  //!< Individual address high byte
constexpr uint16_t AddrIndividualAddressLowByte = 0x118;   //!< Individual address low byte

class EmiKnxIf
{
public:
  EmiKnxIf(int aLedPin);
  void EmiIf_Tasks(void);
  void resetSystemState(void);
  void SetCdcMonMode(bool setreset);
  void SetActivityLed(bool onoff);
  void BlinkActivityLed(void);
  void DoActivityLed(bool LedEnabled);

private:
  int txbuffno;
  uint8_t EmiSystemState;
  uint8_t receivedEmiControlByte; // muss bloederweise für die spätere EMI 1 response gesichert werden
  bool CdcMonActive;
  bool hidIfActive;
  bool procTelWait;
  int ledBlinkCount;
  int ledTimeCount;
  int ledPin;
  uint32_t ledLastDoTime;
  bool ledEnabled;

  void receivedUsbEmiPacket(int buffno);
  uint8_t emiReadOneValue(int memoryAddress);
  void emiWriteOneValue(int addr, uint8_t value, bool &reset);
  void setTPBodyLength(uint8_t *ptr, uint8_t len);

  /**
   * Virtuellen EmiSystemState setzen
   * @brief Wenn der BusMonitor-Modus aktiv ist, dann ist das If selbst immer im Monitor-Modus.
   *        Gegenüber dem Hid-If muss das jedoch gefiltert werden.
   *        Wenn das Hid-If den Monitor-Modus verlangt, muss das natürlich entsprechend umgesetzt werden.
   * @param newValue new system state
   * @param reset is set to to true on reset required.
   */
  void setEmiSystemState(uint8_t newValue, bool &reset);
};

extern EmiKnxIf emiknxif;

#endif /* EMI_KNX_H_ */
