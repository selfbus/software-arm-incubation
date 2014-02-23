/*
 *  eeprom.cpp - Tests for the EEPROM handling
 *
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "catch.hpp"

#include <string.h>
#include <stdio.h>

#include "sblib/eib/user_memory.h"
#include "sblib/eib/bcu.h"
#include "sblib/internal/iap.h"
#include "iap_emu.h"

static const unsigned char pattern[] = {0xCA, 0xFF, 0xEE, 0xAF, 0xFE, 0xDE, 0xAD};
extern unsigned char FLASH[];
#define EEPROM_PAGE_SIZE 0x100

static void checkFlash(byte* address)
{
    for (int i = 0; i < EEPROM_PAGE_SIZE; ++i)
    {
        char c1[4], c2[4];

        sprintf(c1, "%02X", userEeprom[0x100 + i]);
        sprintf(c2, "%02X", address[i]);
        INFO("Byte mismatch at byte position " << i << ": 0x" << c1 << " != 0x" << c2);
        REQUIRE(userEeprom[0x100 + i] == address[i]);
    }
}

TEST_CASE("Test of the basic EEPROM functions","[EEPROM][SBLIB]")
{
    int iap_save [5] ;
    SECTION("Test bus.begin()")
    {
        IAP_Init_Flash(0xFF);
        iapFlashSize();
        memcpy(iap_save, iap_calls, sizeof (iap_calls));
        bcu.begin(0, 0, 0);
        REQUIRE(iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 0));
        REQUIRE(iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
        REQUIRE(iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
        REQUIRE(iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 0));
        REQUIRE(iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 0));
    }

    SECTION("Test bus.end()")
    {
        memcpy(iap_save, iap_calls, sizeof (iap_calls));
        userEeprom.modified();
        bcu.end();
        REQUIRE(iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 1));
        REQUIRE(iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
        REQUIRE(iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
        REQUIRE(iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
        REQUIRE(iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));
    }

}

TEST_CASE("Enhanced EEPROM tests","[EEPROM][SBLIB][ERASE]")
{
    int iap_save [5] ;
    unsigned int i;
    unsigned int ps = sizeof(pattern);
    SECTION("Start with empty FLASH")
    {
        IAP_Init_Flash(0xFF);
        iapFlashSize();
        bcu.begin(0, 0, 0);
        memcpy(iap_save, iap_calls, sizeof (iap_calls));
        for(i=0;i < ps;i++) userEeprom[0x100 + i] = pattern[i];
        userEeprom.modified();
        bcu.end();
        REQUIRE(iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 1));
        REQUIRE(iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
        REQUIRE(iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
        REQUIRE(iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
        REQUIRE(iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));
        checkFlash(FLASH + FLASH_SIZE - SECTOR_SIZE);
    }
    SECTION("Test when first page is valid")
    {
        bcu.begin(0, 0, 0);
        memcpy(iap_save, iap_calls, sizeof (iap_calls));
        for(i=0;i < ps;i++) userEeprom[0x100 + i] = pattern[ps - i];
        userEeprom.modified();
        bcu.end();
        REQUIRE(iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 1));
        REQUIRE(iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
        REQUIRE(iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
        REQUIRE(iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
        REQUIRE(iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));
        checkFlash(FLASH + 0x7100);
    }
    SECTION("Test an overrun of the FLASH area")
    {
        int i;
        for(i = 0; i < 14; i++)
        {
            memcpy(iap_save, iap_calls, sizeof (iap_calls));
            bcu.begin(0, 0, 0);
            userEeprom.modified();
            bcu.end();
            REQUIRE(iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 1));
            REQUIRE(iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
            REQUIRE(iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
            REQUIRE(iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
            REQUIRE(iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));
        }
        memcpy(iap_save, iap_calls, sizeof (iap_calls));
        bcu.begin(0, 0, 0);
        userEeprom.modified();
        bcu.end();
        REQUIRE(iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 2));
        REQUIRE(iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 1));
        REQUIRE(iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 1));
        REQUIRE(iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
        REQUIRE(iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));
    }
}
