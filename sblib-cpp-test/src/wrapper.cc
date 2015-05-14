/*
 *  wrapper.c - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#define private public
#include "sblib/eib/bus.h"
#undef private
#include "sblib/eib/bcu.h"
#include "iap_emu.h"
#include "sblib/internal/variables.h"
#include <sblib/internal/iap.h>
#include <sblib/main.h>
#include <string.h>

extern byte userEepromModified;
extern int sndStartIdx;
extern unsigned int wfiSystemTimeInc;

extern "C"
{
    void queryState (unsigned int * busTelgramLen, unsigned int * sendCurTelegram, unsigned int * sendNextTel, unsigned int * time)
    {
        * busTelgramLen = bus.telegramLen;
        * sendCurTelegram = (bus.sendCurTelegram != NULL);
        * sendNextTel     = (bus.sendNextTel     != NULL);
        * time            = systemTime;
    }

    void copyTelegramIntoRxBuffer(char * telegram, unsigned int length)
    {
        memcpy(bus.telegram, telegram, length);
        bus.telegramLen = length;
        bcu.processTelegram();
    }


    void checkSendTelegram(char * telegram, unsigned int * length)
    {
        bus.timerInterruptHandler();
        * length = bus.sendTelegramLen;
        memcpy(telegram, bus.sendCurTelegram, bus.sendTelegramLen);

        bus.currentByte = SB_BUS_ACK;
        bus.nextByteIndex = 1;
        bus.handleTelegram(true);
    }

    void incSystemTime(unsigned int time)
    {
        systemTime += time;
    }

    void bcu_loop(void)
    {
        LPC_TMR16B1->IR = 0x00;
        bcu.loop();
    }

    void updateUserEeprom(unsigned char * bytes, unsigned int size)
    {
        memcpy(userEepromData, bytes, size);
    }

    void setupTest(void)
    {
        IAP_Init_Flash(0xFF);
        memcpy(FLASH_BASE_ADDRESS + iapFlashSize() - FLASH_SECTOR_SIZE, userEepromData, USER_EEPROM_SIZE);
        sndStartIdx = 0;
        systemTime  = 0;

        wfiSystemTimeInc = 1;
        setup();
        wfiSystemTimeInc = 0;
     }

    void app_loop (void)
    {
        loop ();
    }
}
