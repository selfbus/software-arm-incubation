/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include <sblib/eib/apci.h>
#include <sblib/internal/iap.h>
#include <string.h>
#include <sblib/timeout.h>
#include <sblib/io_pin_names.h>
#include <crc.h>
#include <boot_descriptor_block.h>

/**
 * Updater protocol:
 *   We miss-use the memory write EIB frames. Miss-use because we do not transmitt the address in each request
 *   to have more frame left for the actal data transmission:
 *     BYTES of teh EIB telegram:
 *       8    CMD Nummer (see enum below)
 *       9-x  CMD dependent
 *
 *    UPD_ERASE_SECTOR
 *      9    Number of the sector which should be erased
 *           if the erasing was successful a T_ACK_PDU will be returned, otherwise a T_NACK_PDU
 *    UPD_SEND_DATA
 *      9-   the actual data which will be copied into a RAM buffer for later use
 *           If the RAM buffer is not yet full a T_ACK_PDU will be returned, otherwise a T_NACK_PDU
 *           The address of the RAM buffer will be automatically incremented.
 *           After a Program or Boot Desc Aupdate, the RAM buffer address will be reseted.
 *    UPD_PROGRAM
 *      9-12 How many bytes of the RMA Buffer should be programmed. Be aware that ths value nees to be one of the following
 *           256, 512, 1024, 4096 (required by the IAP of the LPC11xx devices)
 *     13-16 Flash address the data should be programmed to
 *     16-19 The CRC of the data downloaded via the UPD_SEND_DATA commands. If the CRC does not match the
 *           programming returns an error
 *    UPD_UPDATE_BOOT_DESC
 *    UPD_PROGRAM
 *      9-12 The CRC of the data downloaded via the UPD_SEND_DATA commands. If the CRC does not match the
 *           programming returns an error
 *        13 Which boot block should be used
 *    UPD_REQ_DATA
 *      ???
 *    UPD_GET_LAST_ERROR
 *      Returns the reason why the last memory write PDU had a T_NACK_PDU
 *
 *    Workflow:
 *      - erase the sector which needs to be programmed (UPD_ERASE_SECTOR)
 *      - download the data via UPD_SEND_DATA telegrams
 *      - program the transmitted to into the FLASH  (UPD_PROGRAM)
 *      - repeat the above steps until the whole application has been downloaded
 *      - download the boot descriptor block via UPD_SEND_DATA telegrams
 *      - update the boot descriptor block so that the bootloader is able to start the new
 *        application (UPD_UPDATE_BOOT_DESC)
 *      - restart the board (UPD_RESTART)
 */
enum
{
	UPD_ERASE_SECTOR     = 0
,   UPD_SEND_DATA        = 1
,   UPD_PROGRAM          = 2
,   UPD_UPDATE_BOOT_DESC = 3
,   UPD_REQ_DATA         = 10
,   UPD_GET_LAST_ERROR   = 20
,   UPD_SEND_LAST_ERROR  = 21
,   UPD_RESTART          = 30
};

Timeout blinky;

void setup()
{
    bcu.begin(4, 0x2060, 1); // We are a "Jung 2138.10" device, version 0.1
    pinMode(PIN_INFO, OUTPUT);
    pinMode(PIN_RUN,  OUTPUT);
    blinky.start(1);
}

/*
 * The main processing loop.
 */

void loop()
{
    if (blinky.expired())
    {
        if (bcu.connectedTo())
            blinky.start(250);
        else
            blinky.start(1000);
        digitalWrite(PIN_RUN, !digitalRead(PIN_RUN));
    }
    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}

unsigned char ramBuffer[4096];

/*
 * a direct cast does not work due to possible miss aligned addresses.
 * therefore a good old conversion has to be performed
 */
inline unsigned int streamToUIn32(unsigned char * buffer)
{
    return buffer[0] << 24 | buffer [1] << 16 | buffer [2] << 8 | buffer [3];
}

extern const unsigned int __vectors_start__;
extern const unsigned int _etext;
#define ADDRESS2SECTOR(a) ((a + 4095) / 4096)

static bool _prepareReturnTelegram(unsigned int count, unsigned char cmd)
{
    bcu.sendTelegram[5] = 0x63 + count;
    bcu.sendTelegram[6] = 0x42;
    bcu.sendTelegram[7] = 0x40 | count;
    bcu.sendTelegram[8] = cmd;
    return true;
}

/*
 * Checks if the requested sector is allowed to be erased.
 */
inline bool sectorAllowedToErease(unsigned int sectorNumber)
{
    if (sectorNumber == 0) return false; // bootloader sector
    return !(  (sectorNumber >= ADDRESS2SECTOR(__vectors_start__))
            && (sectorNumber <= ADDRESS2SECTOR(_etext))
            );
}

/*
 * Checks if the address range is allowed to be programmed
 */
inline bool addressAllowedToProgram(unsigned int start, unsigned int length)
{
    unsigned int end = start + length;
    if (end <= 4096) return false; // bootloader not allowed to programm!
    return !(  (start >= __vectors_start__)
            && (end   <= _etext)
            );
}

inline bool descriptorAddressValid(unsigned int address)
{
    return true; /// XXX
}

enum UPD_Status
{
      UDP_UNKONW_COMMAND = 0x100
    , UDP_CRC_EROR
    , UPD_ADDRESS_NOT_ALLOWED_TO_FLASH
    , UPD_SECTOR_NOT_ALLOWED_TO_ERASE
    , UPD_RAM_BUFFER_OVERFLOW
    , UPD_WRONG_DESCRIPTOR_BLOCK
    , UPD_APPLICATION_NOT_STARTABLE
    , UDP_NOT_IMPLEMENTED  = 0xFFFF
};

unsigned char handleMemoryRequests(int apciCmd, bool * sendTel, unsigned char * data)
{
    unsigned int count = data[0] & 0x0f;
	unsigned int address;
	static unsigned int ramLocation;
	static unsigned int crc = 0xFFFFFFFF;
	static unsigned int lastError = 0;

    digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
    switch (data [1])
    {
    case UPD_ERASE_SECTOR:
        if (sectorAllowedToErease)
            lastError = iapEraseSector(data [2]);
        else
            lastError = UPD_SECTOR_NOT_ALLOWED_TO_ERASE;
        break;
    case UPD_SEND_DATA:
        if ((ramLocation + count) < sizeof(ramBuffer))
        {
            memcpy((void *)& ramBuffer[ramLocation], data + 2, count);
            crc          = crc32(crc, data + 2, count);
            ramLocation += count;
        }
        else
            lastError = UPD_RAM_BUFFER_OVERFLOW;
        break;
    case UPD_PROGRAM:
        count        = streamToUIn32(data + 2);
        address      = streamToUIn32(data + 2 + 4);
        if (addressAllowedToProgram(address, count))
        {
            if (crc == streamToUIn32(data + 2 + 4 + 8))
            {
                lastError = iapProgram((byte *) address, ramBuffer, count);
            }
            else
                lastError = UDP_CRC_EROR;
        }
        else
            lastError = UPD_ADDRESS_NOT_ALLOWED_TO_FLASH;
        ramLocation = 0;
        crc         = 0xFFFFFFFF;
        break;
    case UPD_UPDATE_BOOT_DESC:
        if (crc == streamToUIn32(data + 2))
        {
            address = 0x1000 - data[6] * 256; // start address of the descriptor block
            if (descriptorAddressValid(address))
            {
                if (checkApplication ((AppDescriptionBlock *) address))
                    lastError = iapProgram((byte *) address, ramBuffer, 256);
                else
                    lastError = UPD_APPLICATION_NOT_STARTABLE;
            }
            else
                lastError = UPD_WRONG_DESCRIPTOR_BLOCK;
        }
        else
            lastError = UDP_CRC_EROR;
         break;
    case UPD_RESTART:
        NVIC_SystemReset();
        break;
    case UPD_REQ_DATA:
        /*
        memcpy(bcu.sendTelegram + 9, (void *)address, count);
        bcu.sendTelegram[5] = 0x63 + count;
        bcu.sendTelegram[6] = 0x42;
        bcu.sendTelegram[7] = 0x40 | count;
        bcu.sendTelegram[8] = UPD_SEND_DATA;
        * sendTel = true;
        * */
        lastError = UDP_NOT_IMPLEMENTED; // set to any error
        break;
    case UPD_GET_LAST_ERROR:
        * sendTel = _prepareReturnTelegram(4, UPD_SEND_LAST_ERROR);
        memcpy(bcu.sendTelegram + 9, (void *)&lastError, 4);
        break;
    default:
        lastError = UDP_UNKONW_COMMAND; // set to any error
    }
    if (lastError == IAP_SUCCESS)
        return T_ACK_PDU;
    return T_NACK_PDU;
}
