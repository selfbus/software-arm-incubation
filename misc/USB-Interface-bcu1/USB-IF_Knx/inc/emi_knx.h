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

#include <sblib/eib/bcu1.h>
#include "device_mgnt_const.h"

/**
 * Layer access in EMI 1
 *
 * @brief KNX Spec. 2.1 3/6/3 3.1.3 p.14
 */
enum SystemState
{
    BusMonitor = 0x90,       //!< BCU_STATUS_SERIAL_PEI
    LinkLayer = 0x12,        //!< BCU_STATUS_SERIAL_PEI, BCU_STATUS_LINK_LAYER (wird von der ETS genutzt)
    TransportLayer = 0x96,   //!< BCU_STATUS_SERIAL_PEI, BCU_STATUS_LINK_LAYER, BCU_STATUS_TRANSPORT_LAYER
    ApplicationLayer = 0x1E, //!< BCU_STATUS_SERIAL_PEI, BCU_STATUS_LINK_LAYER, BCU_STATUS_TRANSPORT_LAYER, BCU_STATUS_APPLICATION_LAYER
    Reset = 0xC0,            //!< BCU_STATUS_DOWNLOAD_MODE (ist der Default- und Reset-Zustand)
};

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
    EmiKnxIf(void) = delete;

    /**
     * Constructs an EmiKnxIf object.
     *
     * @details It puts the BCU into download mode, which disables all KNX bus communication.
     *
     * @param bcu Pointer to a BCU1 object that represents the KNX bus access controller.
     * @param aLedPin The GPIO pin used to control the activity LED.
     *
     * @warning bcu must already be started with bcu.begin().
     *          Otherwise @ref SetCdcMonMode may hang in `while (emiBcu->bus->sendingFrame())`,
     *          because bus->sendCurTelegram may be uninitialized.
     */
    EmiKnxIf(BCU1 * bcu, int aLedPin);
    void EmiIf_Tasks(void);

    /**
     *  Set bcu in download mode to disable all KNX-Bus communication
     */
    void reset(void);
    void SetActivityLed(bool onoff);
    void BlinkActivityLed(void);
    void DoActivityLed(bool LedEnabled);
    void setDeviceMode(DeviceMode newDeviceMode);

private:
    BCU1 * emiBcu = nullptr;

    int txbuffno = -1;
    uint8_t receivedEmiControlByte = 0; // muss bloederweise für die spätere EMI 1 response gesichert werden
    int ledBlinkCount = 0;
    int ledTimeCount = 0;
    int ledPin = 0;
    uint32_t ledLastDoTime = 0;
    bool ledEnabled = true; // Damit die LED beim Start AUSgeschaltet wird
    DeviceMode deviceMode = DeviceMode::Invalid;

    void receivedUsbEmiPacket(int buffno);
    void sendReceivedTelegramAsEMI(uint8_t * telegram, uint8_t length);

    uint8_t emiReadOneValue(int memoryAddress);
    void emiWriteOneValue(int addr, uint8_t value, bool &reset);
    void setTPBodyLength(uint8_t *ptr, uint8_t len);

    uint8_t getSystemState();
    void setSystemState(const uint8_t newStatus);

    bool getBcuLayerState(const uint8_t layer);
    void setBcuLayerState(const uint8_t layer, const bool newState);

    void handleTelegramForUs(uint8_t * telegram, uint8_t lenght);
    bool isKNXenabled();
};

#endif /* EMI_KNX_H_ */
