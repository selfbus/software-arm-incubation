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

enum
{
	UPD_ERASE_SECTOR
,   UPD_SET_ADDR
,   UPD_SEND_DATA
,   UPD_PROGRAM
,   UPD_REQ_DATA
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

inline unsigned int uint32(unsigned char * buffer)
{
    return buffer[0] << 24 | buffer [1] << 16 | buffer [2] << 8 | buffer [3];
}

unsigned char handleMemoryRequests(int apciCmd, bool * sendTel, unsigned char * data)
{
	unsigned char result = 0;
    unsigned int count = data[0] & 0x0f;
	static unsigned int address = 0x3000;
	static unsigned int ramLocation;
	static unsigned int crc = 0xFFFFFFFF;

    digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
    if (apciCmd == APCI_MEMORY_WRITE_PDU)
    {
        IAP_Status iap = IAP_SUCCESS;
    	switch (data [1])
    	{
    	case UPD_ERASE_SECTOR:
    		iap = iapEraseSector(data [2]);
    		break;
    	case UPD_SET_ADDR:
    	    address     = uint32(data + 2);
            ramLocation = 0;
    		crc         = 0xFFFFFFFF;
    		break;
    	case UPD_SEND_DATA:
    		memcpy((void *)& ramBuffer[ramLocation], data + 2, count);
    		crc          = crc32(crc, data + 2, count);
    		ramLocation += count;
    		break;
    	case UPD_PROGRAM:
    	    count        = uint32(data + 2);
            if (crc == uint32(data + 2 + 4))
            {
                iap = iapProgram((byte *) address, ramBuffer, count);
            }
            else
                iap = IAP_COMPARE_ERROR;
    		ramLocation = 0;
            crc         = 0xFFFFFFFF;
    		break;
    	default:
    	    iap = IAP_INVALID_COMMAND; // set to any error
    	}
    	if (iap == IAP_SUCCESS)
    	    result = T_ACK_PDU;
    	else
            result = T_NACK_PDU;
    }

    if (apciCmd == APCI_MEMORY_READ_PDU)
    {
    	switch (data [1])
    	{
    	case UPD_REQ_DATA:
    		memcpy(bcu.sendTelegram + 9, (void *)address, count);
            bcu.sendTelegram[5] = 0x63 + count;
            bcu.sendTelegram[6] = 0x42;
            bcu.sendTelegram[7] = 0x40 | count;
            bcu.sendTelegram[8] = UPD_SEND_DATA;
    		* sendTel = true;
    		break;
    	default:
        	result = T_NACK_PDU;
    	}
    }
	return result;
}
