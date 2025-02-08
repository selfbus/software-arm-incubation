/*
 *  device_mgnt_const.h - Device management, mode management and mode switching
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
 
#ifndef DEVICE_MGNT_CONST_H_
#define DEVICE_MGNT_CONST_H_

#include <stdint.h>

constexpr uint32_t C_Dev_Baurate = 115200; //!> Baudrate for the inter-mcu communication

constexpr uint8_t cdc_OffSet = 3; ///\todo find better name

///\todo verify docu
// Inter-mcu device management protocol
// Byte     Description
//  0       Packet length
//  1       1 Byte Checksumme, alle Bytes des Telegrams addiert (incl Checksumme), modulo 256, ergeben 255 from BufferMgr.h
//  2       HID Report Header - HRH (see knxusb_const.h for details (e.g. C_HRH_IdDev)
//  3       C_Dev main command/info?
//  4       C_DevSys_xx sub command/info?
//  5       UNKNOWN, in "void DeviceManagement::SysIf_Tasks(bool UsbActive)" it is not set, yet packet len

///\todo packet length 5 is right now an assumption based on
///      USB-IF_Usb => void DeviceManagement::SysIf_Tasks(bool UsbActive)
///      USB-IF_Knx => void DeviceManagement::DevMgnt_Tasks(void)
constexpr uint8_t C_Dev_Packet_Length = 5; // searched for (2+3)

#define C_Dev_Idle       1 //!< Inter-mcu communication heartbeat?
#define C_Dev_Sys        2 //!< Mode control command. See @ref DeviceMode for possible device modes

/**
 * Set serial control lines (RTS/CTS...)
 *
 * @details USB mcu sends it to set the soft uart (prog_uart) control lines.<br>
 *          KNX mcu sends it to acknowledge that a data packet was sent to the soft uart (prog_uart)
 */
#define C_Dev_Isp        3

//#define C_TxTimeout 450
#define C_RxTimeout 450
#define C_IdlePeriod 200 //!> Idle heartbeat interval in milliseconds (inter-mcu communication)

enum class DeviceMode : uint8_t
{
    Invalid = 0,    //!< Invalid mode
    Halt = 1,       //!< Initialization or USB stopped working
    HidOnly = 2,    //!< KNX-Interface only, KNX bus interface for ETS, knxd or Updater.
    UsbMon = 3,     //!< USB-Monitor, sends raw EMI 1 messages to virtual serial port of the USB port.
    BusMon = 4,     //!< KNX-Busmonitor, sends decoded KNX TP1 telegrams to virtual serial port of the USB port.

    /**
     * ISP-Programmer for KNX bus access mcu (TS_ARM connected at P1).
     *
     * @note Jumper JP5 (PIO 1_19) must be closed on startup.
     */
    ProgBusChip = 5,

    /**
     * ISP-Programmer for user mcu (connected at Prog-If (P3)).
     *
     * @note Jumper JP2 (PIO 0_3) and JP6 (3.3V)  must be closed.
     */
    ProgUserChip = 6
};

#endif /* DEVICE_MGNT_CONST_H_ */
