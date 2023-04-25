/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eibBCU1.h>
#include <sblib/io_pin_names.h>
#include <sblib/timeout.h>
#include "GenFifo.h"
#include "BufferMgr.h"
#include "UartIf.h"
#include "Emi_Knx.h"
#include "device_mgnt.h"
#include "prog_uart.h"

// Code Protection deaktiviert
__attribute__ ((used,section(".crp"))) const unsigned int CRP_WORD = 0xFFFFFFFF;

BCU1 bcu = BCU1();

APP_VERSION("SBif_knx", "2", "10")

BcuBase* setup()
{
    pinMode(PIN_PROG, OUTPUT);
    digitalWrite(PIN_PROG, false);
    pinMode(PIO1_5, OUTPUT);
    digitalWrite(PIO1_5, false);
    delay(500);
    digitalWrite(PIN_PROG, true);
    digitalWrite(PIO1_5, true);
    bcu.begin(2, 1, 1); // ABB, dummy something device

    /* ///\todo why? bcu.begin checks for 0 and sets to default KNX address 15.15.255, 1.1.1 will most likely already be used
    int addr = bcu.ownAddress();
    if ((addr == 0) || (addr == 0xffff))
    {
        bcu.setOwnAddress(0x1101); // default 1.1.1
    }
     */
    uart.Init(115200, false);
    return (&bcu);
}

/*
 * Es gibt laut HandleTelegramm() irgendwelche Acks < 8 Bytes, dies scheinen jedoch nicht die
 * zu sein, die mich interessieren.
 * Alle "interessanten" Telegramme sind >= 8 Bytes lang. Über USB werden sie ohne Checksumme getunnelt,
 * dort sind sie also >= 7 Bytes lang.
 * Es ist möglich, alle Telegramme >= 8 Bytes an einem Monitor vorbeizuschleusen, indem:
 * In bcu.userRam->status() das Bit BCU_STATUS_TRANSPORT_LAYER gelöscht wird. Dann ruft BcuBase.loop() nie processTelegram()
 * auf und die Telegramme sind noch vorhanden, wenn loop() der Applikation aufgerufen wird.
 * Dort kann dann das Telegramm kopiert und danach bei Bedarf processTelegramm() aufgerufen werden. Muss das
 * überhaupt? Es wäre praktisch, wenn man über den Bus die Adresse schreiben kann, insofern also doch?
 * Wenn Bit BCU_STATUS_TRANSPORT_LAYER gelöscht wird, MUSS auch BCU_STATUS_LINK_LAYER gelöscht werden,
 * ansonsten wird jedes Telegram von uns mit einem LL_ACK beantwortet.
 */
/*
 * The main processing loop.
 */
void loop()
{
  uart.SerIf_Tasks();
  // emiknxif.SetCdcMonMode(true); //only for debug to "hard" activate busmonitor mode
  emiknxif.EmiIf_Tasks();
  proguart.SerIf_Tasks();
  devicemgnt.DevMgnt_Tasks();
  // Sleep until the next 1 msec timer interrupt occurs (or shorter)
  __WFI();
}

void loop_noapp()
{
  loop();
}
