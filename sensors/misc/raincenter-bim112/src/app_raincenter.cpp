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


enum ePollState {Idle, PolledParam, ReceivedParam, PolledDisplay, ReceivedDisplay};

static ePollState PollState = Idle;     // holds the actual state of the state machine
static Timeout RaincenterPollTimer;     // Timeout Timer to cyclic poll the Raincenter
static Timeout RaincenterDelayTxTimer;  // Timeout Timer to delay serial send messages, because raincenter needs som time between messages send to him
static RCParameterMessage rcParamMsg;   // holds the last received RCParameterMessage
static RCDisplayMessage  rsDisplayMsg;  // holds the last receive RCDisplayMessage
static int rcMessageLengthWaitingFor;   // holds the length of the serial message we are waiting for

bool Write2Serial(byte ch)
{
    if (!RaincenterDelayTxTimer.stopped())
    {
        while (!RaincenterDelayTxTimer.expired())
        {
            delay(1);
        }
    }
    FlashTX_LED();
    return (serial.write(ch) == 1);
}


bool RunPollStateMachine(int SerialBytesAvailable)
{
    bool result = false;
    static byte * rx;

    if ((SerialBytesAvailable > 0) && (rcMessageLengthWaitingFor > 0) && (SerialBytesAvailable == rcMessageLengthWaitingFor))
    {
        if (SerialBytesAvailable == rcMessageLengthWaitingFor)
        {
            rx = new byte[SerialBytesAvailable];
            serial.readBytes(&rx[0], SerialBytesAvailable);
            if (SerialBytesAvailable == RCParameterMessage::msgLength)
            {
                RCParameterMessage msg;
                if (msg.Decode(&rx[0], SerialBytesAvailable))
                {
                    PollState = ReceivedParam;
                }
            }
            else if (SerialBytesAvailable == RCDisplayMessage::msgLength)
            {
                RCDisplayMessage msg;
                if (msg.Decode(&rx[0], SerialBytesAvailable))
                {
                    PollState = ReceivedDisplay;
                }
            }
            delete[] rx;
        }
    }

    switch (PollState)
    {
        case Idle:
            // seams to be the first run, so lets start polling with RCParametermessage
            serial.clearBuffers();
            rcMessageLengthWaitingFor = RCParameterMessage::msgLength;
            Write2Serial(RCParameterMessage::msgIdentifier);
            PollState = PolledParam;
            RaincenterPollTimer.start(POLL_INTERVAL_MS);
            result = true;
            break;
        case PolledParam:
            // seams like we got no valid answer, so lets just reset
            if (RaincenterPollTimer.expired())
            {
                PollState = Idle;
                RaincenterPollTimer.start(POLL_INTERVAL_MS);
            }
            result = false;
            break;
        case ReceivedParam:
            // ok, we received the RCParametermessage, now lets get the RCDisplaymessage
            serial.clearBuffers();
            rcMessageLengthWaitingFor = RCDisplayMessage::msgLength;
            Write2Serial(RCDisplayMessage::msgIdentifier);
            PollState = PolledDisplay;
            RaincenterDelayTxTimer.start(POLL_INTERVAL_MS);
            RaincenterPollTimer.start(POLL_INTERVAL_MS);
            result = true;
            break;
        case PolledDisplay:
            // seams like we got no valid answer, so lets just reset
            if (RaincenterPollTimer.expired())
            {
                PollState = Idle;
                RaincenterPollTimer.start(POLL_INTERVAL_MS);
            }
            result = false;
            break;
        case ReceivedDisplay:
            //
            serial.clearBuffers();
            FlashRX_LED(RX_FLASH_OK_MS);
            PollState = Idle;
            result = true;
            break;
        default:
            //
            break;
    } //switch
    return result;
}

void objectUpdated(int objno)
{

}

void checkPeriodic(void)
{
    if (serial.available() > 0) // check weather we got something on the serial
    {
        RaincenterDelayTxTimer.start(RAINCENTER_TX_DELAY_SEND_MS); // delay the next possible serial send, cause raincenter needs a little break
        FlashRX_LED();
    }
    RunPollStateMachine(serial.available());

    debugCheckPeriodic(); // call to switch off debugging TX/RX Leds
}

void initApplication(void)
{
    // serial port initialization 2400 8N1
    serial.setTxPin(RC_TX_PIN);
    serial.setRxPin(RC_RX_PIN);
    serial.begin(RAINCENTER_BAUDRATE);
    PollState = Idle;
    rcMessageLengthWaitingFor = -1;
    RaincenterPollTimer.start(1);
    RaincenterDelayTxTimer.start(1);

#ifdef DEBUG
    // LED Initialize
    pinMode(TX_LED, OUTPUT);
    pinMode(RX_LED, OUTPUT);

    // LED Set Initial Value (ON|OFF)
    digitalWrite(TX_LED, LED_ON);
    digitalWrite(RX_LED, LED_ON);
    delay(LED_STARTUP_DISPLAY);
    digitalWrite(TX_LED, LED_OFF);
    digitalWrite(RX_LED, LED_OFF);
#endif
}
