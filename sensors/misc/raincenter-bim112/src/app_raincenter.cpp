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
static Timeout SendPeriodicTimer;       // TODO this is just for testing, Timeout Timer to send all objects periodically
static Timeout RaincenterPollTimer;     // Timeout Timer to cyclic poll the Raincenter
static Timeout RaincenterDelayTxTimer;  // Timeout Timer to delay serial send messages, because raincenter needs som time between messages send to him
static RCParameterMessage rcParamMsg;   // holds the last received RCParameterMessage
static RCDisplayMessage  rcDisplayMsg;  // holds the last receive RCDisplayMessage
static int rcMessageLengthWaitingFor;   // holds the length of the serial message we are waiting for
static char CommandWaitingForSend = RC_INVALID_COMMAND; // holds the command which we want send between polling (Pollstate = Idle)

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
                    if (rcParamMsg != msg)
                    {
                        // TODO check weather to send or not conditions
                        // Parameters have changed, lets send them for now
                        rcParamMsg = msg;
                        objectWrite(20, rcParamMsg.Level_m3_Calibrated()*10);
                    }
                    PollState = ReceivedParam;
                }
            }
            else if (SerialBytesAvailable == RCDisplayMessage::msgLength)
            {
                RCDisplayMessage msg;
                if (msg.Decode(&rx[0], SerialBytesAvailable))
                {
                    rcDisplayMsg = msg;
                    if (rcDisplayMsg != msg)
                    {
                        // TODO check weather to send or not conditions
                        // Parameters have changed, lets send them for now
                        objectWrite(17, rcDisplayMsg.IsSwitchedToTapWater());
                    }
                    PollState = ReceivedDisplay;
                }
            }
            delete[] rx;
        }
    }

    switch (PollState)
    {
        case Idle:
            if (CommandWaitingForSend != RC_INVALID_COMMAND)
            {
                RaincenterPollTimer.start(1000);
                Write2Serial(CommandWaitingForSend);
                CommandWaitingForSend = RC_INVALID_COMMAND;
                break;
            }

            if (RaincenterPollTimer.expired() || RaincenterPollTimer.stopped())
            {
                serial.clearBuffers();
                rcMessageLengthWaitingFor = RCParameterMessage::msgLength;
                Write2Serial(RCParameterMessage::msgIdentifier);
                PollState = PolledParam;
                RaincenterPollTimer.start(REPOLL_INTERVAL_MS);
                result = true;
            }
            break;
        case PolledParam:
            // seams like we got no valid answer, so lets just reset
            if (RaincenterPollTimer.expired())
            {
                PollState = Idle;
                RaincenterPollTimer.start(REPOLL_INTERVAL_MS);
            }
            result = false;
            break;
        case ReceivedParam:
            // ok, we received the RCParametermessage, now lets get the RCDisplaymessage
            serial.clearBuffers();
            rcMessageLengthWaitingFor = RCDisplayMessage::msgLength;
            RaincenterDelayTxTimer.start(RAINCENTER_TX_DELAY_SEND_MS); // delay the next serial send
            Write2Serial(RCDisplayMessage::msgIdentifier);
            PollState = PolledDisplay;
            RaincenterPollTimer.start(REPOLL_INTERVAL_MS);
            result = true;
            break;
        case PolledDisplay:
            // seams like we got no valid answer, so lets just reset
            if (RaincenterPollTimer.expired())
            {
                PollState = Idle;
                RaincenterPollTimer.start(REPOLL_INTERVAL_MS);
            }
            result = false;
            break;
        case ReceivedDisplay:
            //
            serial.clearBuffers();
            FlashRX_LED(RX_FLASH_OK_MS);
            RaincenterPollTimer.start(POLL_INTERVAL_MS);
            PollState = Idle;
            result = true;
            break;
        default:
            // this should never happen
            serial.clearBuffers();
            PollState = Idle;
            result = false;
            break;
    } //switch
    return result;
}

void objectUpdated(int objno)
{
    if (objno == 13)
    {
        bool switchToTapWater;
        switchToTapWater = objectRead(13);
        if (switchToTapWater)
        {
            CommandWaitingForSend = RCSwitchToTapWaterRefillMessage::msgIdentifier;
        }
        else
        {
            CommandWaitingForSend = RCSwitchToReservoirMessage::msgIdentifier;
        }
    }
}

void checkPeriodic(void)
{
    if (serial.available() > 0) // check weather we got something on the serial
    {
        RaincenterDelayTxTimer.start(RAINCENTER_TX_DELAY_SEND_MS); // delay the next possible serial send, cause raincenter needs a little break
        FlashRX_LED();
    }
    RunPollStateMachine(serial.available());

    // TODO this is just for testing
    if (SendPeriodicTimer.expired())
    {

        objectWrite(20, rcParamMsg.Level_m3_Calibrated()*10);
        objectWrite(17, rcDisplayMsg.IsSwitchedToTapWater());
        SendPeriodicTimer.start(SENDPERIODIC_INTERVAL_MS);
    }

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
    // TODO this is just for testing
    SendPeriodicTimer.start(SENDPERIODIC_INTERVAL_MS);

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
