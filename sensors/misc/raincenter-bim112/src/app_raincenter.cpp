/*
 *  app_raincenter.cpp
 *
 *  Created on: 11.09.2020
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/internal/iap.h>
#include <sblib/eib.h>
#include <sblib/timeout.h>
#include <sblib/serial.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "app_raincenter.h"
#include "rc_protocol.h"
#include "debug.h"



static Timeout RaincenterPollTimer;

void objectUpdated(int objno)
{

}

void checkPeriodic(void)
{

    static int rxBytesRdy2Read;
    static byte * rx;
    static RCParameterMessage msg;

    if (RaincenterPollTimer.expired())
    {
        FlashTX_LED();
        RaincenterPollTimer.start(POLL_INTERVAL_MS);
        serial.write(msg.msgIdentifier);
    }

    rxBytesRdy2Read = serial.available();
    if (rxBytesRdy2Read >= msg.msgLength)
    {
        FlashRX_LED();
        rx = new byte[rxBytesRdy2Read];
        serial.readBytes(&rx[0], rxBytesRdy2Read);
        // serial.write(&rx[0], rxBytesRdy2Read);
        msg.Decode(&rx[0], rxBytesRdy2Read);

        delete[] rx;
    }

    debugCheckPeriodic(); // call to switch off TX/RX Leds
}

void initApplication(void)
{
    // serial port initialization 2400 8N1
    // serial.setTxPin(RC_TX_PIN);
    // serial.setRxPin(RC_RX_PIN);
    serial.begin(RAINCENTER_BAUDRATE);
    RaincenterPollTimer.start(POLL_INTERVAL_MS);

#ifdef DEBUG
    // LED Initialize
    pinMode(TX_LED, OUTPUT);
    pinMode(RX_LED, OUTPUT);

    // LED Set Initial Value (ON|OFF)
    digitalWrite(TX_LED, LED_ON);
    digitalWrite(RX_LED, LED_ON);
    delay(500);
    digitalWrite(TX_LED, LED_OFF);
    digitalWrite(RX_LED, LED_OFF);
#endif
}
