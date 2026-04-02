/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef HOERMANN_PROTOCOL_H_
#define HOERMANN_PROTOCOL_H_

#include <cstdint>


/**
 * Addresses used on the Hoermann RS485 bus.
 *
 * The bus uses 8-bit addresses to identify participants.
 * Address ranges are defined as follows (source:
 * https://blog.bouni.de/posts/2018/hoerrmann-uap1/):
 *
 * | Address(es) | Assignment                         |
 * |------------:|------------------------------------|
 * |           0 | Broadcast                          |
 * |      1 – 15 | Reserved                           |
 * |     16 – 45 | Intelligent control panels         |
 * |     46 – 47 | Not specified in the patent        |
 * |     48 – 79 | Dynamically assigned by the master |
 * |    80 – 100 | Not specified in the patent        |
 * |   101 – 109 | Slave drives                       |
 * |   110 – 127 | Reserved                           |
 * |         128 | Master drive                       |
 * |   129 – 143 | Intelligent controllers            |
 * |         144 | Diagnostic device / PC             |
 * |   145 – 255 | Reserved                           |
 */
enum BusAddress: uint8_t
{
    broadCastAddress = 0x00, //!< Broadcast address, message is received by all bus participants
    uap1Address      = 0x28, //!< Address 40: UAP1 intelligent control panel
    masterAddress    = 0x80  //!< Address 128: Master drive (e.g. SupraMatic)
};

/**
 * Commands exchanged between devices on the bus.
 *
 * These commands are carried in the data section of a bus frame
 * and control the interaction between master and devices.
 */
enum class DeviceCommand: uint8_t
{
    /**
     * Master scans for devices on the bus.
     * Devices reply with their type and address
    **/
    scan           = 0x01,
    statusRequest  = 0x20, //!< Master requests the current status from a device
    statusResponse = 0x29  //!< Device responds with its status
};

/**
 * Known device types on the Hoermann RS485 bus.
 *
 * The device type is sent as part of the scan response so the
 * master can identify the kind of device at a given address.
 */
enum DeviceType: uint8_t
{
    uap1Device = 0x14 //!< UAP1 intelligent control panel (device type 20)
};

/**
 * Byte offset of the bus address
**/
constexpr uint8_t ADDRESS_OFFSET = 0;

/**
 * Byte offset of the counter (high nibble) and data length (low nibble)
**/
constexpr uint8_t COUNT_LENGTH_OFFSET = 1;

/**
 * Byte offset where the command / data payload begins
**/
constexpr uint8_t DATA_OFFSET = 2;

#endif /* HOERMANN_PROTOCOL_H_ */
