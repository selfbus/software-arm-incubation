/*
 *  app_main.cpp - The ft12 application's main.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>
#include <sblib/timeout.h>
#include <sblib/eib/apci.h>
#include <sblib/serial.h>
#include <sblib/version.h>
#include "bcu_ft12.h"
#include "ft12_protocol.h"

#define LED_SERIAL_RX               (PIO0_6) //!< Serial-Rx LED Pin
#define LED_KNX_RX                  (PIO0_7) //!< KNX-Rx LED Pin

#define PIN_FT_SERIAL_TX            (PIN_TX) //!< Serial-Tx Pin
#define PIN_FT_SERIAL_RX            (PIN_RX) //!< Serial-Rx Pin

#define LED_SERIAL_RX_BLINKTIME     (50)    //!< Receiving serial data blinking timeout in milliseconds
#define LED_KNX_RX_BLINKTIME        (100)    //!< Receiving KNX packets blinking timeout in milliseconds

#define FT_OWN_KNX_ADDRESS          (0x11fe) //!< Our own knx-address: 1.1.254
#define FT_FRAME_SIZE               (32)     //!< Maximum size of FT1.2 frames
#define FT_MAX_SEND_RETRY           (1)      //!< Do not repeat sending
#define FT_BAUDRATE                 (19200)  //!< Ft12 baudrate
#define FT_TIMEOUT_MS               (510*1000/FT_BAUDRATE) //!< Timeout of the serial communication (~510 bits)

APP_VERSION("SBft12  ", "0", "01")

BcuFt12 bcu = BcuFt12();  //!< Bus coupling unit Maskversion 0x0012 of the ft12 module

byte ftFrameIn[FT_FRAME_SIZE];  //!< Buffer for incoming FT1.2 frames
byte ftFrameOut[FT_FRAME_SIZE]; //!< Buffer for outgoing FT1.2 frames

static unsigned int lastSerialRecvTime = 0;

byte* telegramOut1; //!< 1.Buffer for outgoing telegrams
byte* telegramOut2; //!< 2.Buffer for outgoing telegrams

int telegramOutId; //!< Index of the next buffer for an outgoing knx telegram

uint8_t ftFrameInLen;       //!< Length of the data in ftFrameIn
uint8_t ftFrameOutLen;      //!< Length of the data in ftFrameOut

bool sendFrameCountBit;
bool rcvFrameCountBit;
int16_t lastChecksum;

Timeout knxRxTimeout;       //!< KNX-Rx LED blinking timeout
Timeout knxSerialTimeout;   //!< Serial-Rx LED blinking timeout

FtFrameType frameType = FT_NONE;


void debugFatal()
{
#ifdef DEBUG
    bcu.setProgPin(LED_KNX_RX);
    fatalError();
#endif
}

/**
 * Reset the buffers.
 */
void reset()
{
    serial.clearBuffers();
    sendFrameCountBit = true;
    sendFrameCountBit = true;
    lastChecksum = -1;
    ftFrameInLen = 0;
    ftFrameOutLen = 0;
    frameType = FT_NONE;
}

/**
 * Initialize the application.
 */
BcuBase* setup()
{
    telegramOut1 = new byte(bcu.maxTelegramSize());
    telegramOut2 = new byte(bcu.maxTelegramSize());

    pinMode(LED_KNX_RX, OUTPUT);    // KNX-Rx LED
    digitalWrite(LED_KNX_RX, false);
    pinMode(LED_SERIAL_RX, OUTPUT); // Serial-Rx LED
    digitalWrite(LED_SERIAL_RX, false);

    bcu.begin();
    bcu.setOwnAddress(FT_OWN_KNX_ADDRESS);
    bcu.bus->maxSendTries(FT_MAX_SEND_RETRY);
    bcu.userRam->status() ^= BCU_STATUS_TRANSPORT_LAYER | BCU_STATUS_PARITY;

    // Disable telegram processing by the lib
    if (bcu.userRam->status() & BCU_STATUS_TRANSPORT_LAYER)
    {
        bcu.userRam->status() ^= BCU_STATUS_TRANSPORT_LAYER | BCU_STATUS_PARITY;
    }

    serial.setTxPin(PIN_FT_SERIAL_TX);
    serial.setRxPin(PIN_FT_SERIAL_RX);
    serial.begin(FT_BAUDRATE, SERIAL_8E1);
    reset();
    telegramOutId = 0;
    return (&bcu);
}

/**
 * Send a FT frame of variable length. The frame buffer must have enough space
 * so that the checksum and end byte are added.
 *
 * @param frame          The buffer that contains the frame
 * @param funcCode       The function code, e.g. FC_SEND_UDAT
 * @param emi            The @ref EmiCode to send
 * @param userDataLength The length of the frame's payload
 * @param frameCountBit  Frame count bit of the control byte
 */
void sendVariableFrame(byte* frame, const FtFunctionCode& funcCode, const EmiCode& emi, const uint8_t& userDataLength,
        const bool& frameCountBit)
{
    frame[4] = 0xD0;
    if (frameCountBit)
    {
        frame[4] |= 0x20;
    }

    frame[0] = FT_VARIABLE_START;
    frame[1] = userDataLength;
    frame[2] = userDataLength;
    frame[3] = FT_VARIABLE_START;
    frame[4] |= (funcCode & 0x0f);
    frame[5] = emi;

    frame[userDataLength + 4] = calcCheckSum(frame, userDataLength);
    frame[userDataLength + 5] = FT_END;
    serial.write(frame, userDataLength + 6);
}

/**
 * Process a fixed length FT frame
 *
 * @param frame - 4 byte Buffer containing the fixed length frame to process
 * @note KNX Spec. 2.1 3/6/2 6.4.3.2 p.23ff
 */
bool processFixedFrame(uint8_t* frame)
{
    FtControlField cf  = controlFieldFromByte(frame[1]);

    if (!cf.isRequest)
    {
        return false;
    }

    switch (cf.functionCode)
    {
        case FC_SEND_RESET:
            reset();
            serial.write(FT_ACK);
            return true;
            break;

        case FC_SEND_UDAT:
            return false; ///\todo FC_SEND_UDAT
            break;
        case FC_REQ_STATUS:
            return false; ///\todo FC_REQ_STATUS
            break;

        default:
            serial.write(FT_ACK);
            return false;
    }
    return true;
}

/**
 * Process a L_DataConnected request in ftFrameIn[]
 */
void processDataConnectedRequest()
{
    for (int i = 6; i < 10; ++i)
        ftFrameOut[i] = 0;

    int apci = makeWord(ftFrameIn[12], ftFrameIn[13]);
    switch (apci)
    {
    case APCI_DEVICEDESCRIPTOR_READ_PDU:
        uint16_t version = 0x0012;
        serial.write(FT_ACK);
        ftFrameOut[11] = 0x63; // DRL 3 bytes
        ftFrameOut[12] = HIGH_BYTE(APCI_DEVICEDESCRIPTOR_RESPONSE_PDU);
        ftFrameOut[13] = lowByte(APCI_DEVICEDESCRIPTOR_RESPONSE_PDU);
        ftFrameOut[14] = HIGH_BYTE(version);
        ftFrameOut[15] = lowByte(version);
        sendVariableFrame(ftFrameOut, FC_SEND_UDAT, T_Data_Connected_Con, 12, true);
        break;
    }
}

/**
 * Process a variable length FT frame
 */
bool processVariableFrame(uint8_t* frame, uint8_t length)
{
    byte* telegramOut;
    FtControlField cf  = controlFieldFromByte(frame[4]);

    if (cf.functionCode != FC_SEND_UDAT)
    {
        return false;
    }

    EmiCode emi = (EmiCode)frame[5]; //1. PEI_Switch_Req
    rcvFrameCountBit = (ftFrameIn[4] >> 5) & 0x1;
    switch (emi)  // EMI code
    {
    case PEI_Identify_Req:
        serial.write(FT_ACK);
        ftFrameOut[6]  = HIGH_BYTE(bcu.ownAddress());
        ftFrameOut[7]  = lowByte(bcu.ownAddress());
        ftFrameOut[8]  = 0x00;
        ftFrameOut[9]  = 0x01;
        ftFrameOut[10] = 0x00;
        ftFrameOut[11] = 0x01;
        ftFrameOut[12] = 0xE4;
        ftFrameOut[13] = 0x5A;
        ftFrameOut[14] = 0;
        sendVariableFrame(ftFrameOut, FC_SEND_UDAT, PEI_Identify_Con, 10, sendFrameCountBit);
        break;

    case PEI_Switch_Req: // KNX Spec. 3/6/3 3.4.1 p.14
        reset();
        serial.write(FT_ACK);
        break;

    case T_Connect_Req:
        serial.write(FT_ACK);
        ftFrameOut[6]  = 0;
        ftFrameOut[7]  = frame[9];
        ftFrameOut[8]  = frame[10];
        ftFrameOut[9]  = 0;
        ftFrameOut[10] = 0;
        ftFrameOut[11] = 0;
        sendVariableFrame(ftFrameOut, FC_SEND_UDAT, T_Connect_Con, 7, sendFrameCountBit);
        break;

    case T_Data_Connected_Req:
        processDataConnectedRequest();
        break;

    case L_Data_Req: // KNX Spec. 2.1 3/6/3 3.3.4.2 p.20
    {
        serial.write(FT_ACK);
        uint8_t userDataLength = frame[1];
        if (telegramOutId)
            telegramOut = telegramOut1;
        else telegramOut = telegramOut2;
        telegramOutId = !telegramOutId;

        for (uint8_t i = 3; i < userDataLength - 2; ++i)
        {
            telegramOut[i] = ftFrameIn[i + 6];
        }

        uint8_t priority = (frame[6] &0x0C); // requested priority
        telegramOut[0] = 0xB0 | (frame[6] & 0x0F); // control byte
        ftFrameOut[6]  = priority & 0xfe; // return requested priority and a positive ACK (last bit 0)
        for (uint8_t i = 7; i < length - 2; ++i)
        {
            ftFrameOut[i] = ftFrameIn[i];
        }

        bcu.bus->sendTelegram(telegramOut, userDataLength - 2);
        sendVariableFrame(ftFrameOut, FC_SEND_UDAT, L_Data_Con, userDataLength, sendFrameCountBit);
        // serial.flush();
        break;
    }

    default:
        return false;
    }
    return true;
}

/**
 * Send a telegram of the bus as variable frame.
 */
void processTelegram()
{
    for (uint8_t i = 0; i < bcu.bus->telegramLen - 1; i++)
    {
        ftFrameOut[i + 6] = bcu.bus->telegram[i];
    }
    ftFrameOut[4] = 0xf0;
    sendVariableFrame(ftFrameOut, FC_SEND_UDAT, L_Data_Ind, bcu.bus->telegramLen + 1, sendFrameCountBit);
}

static void timeoutSerial()
{
    // Timeout, discard any received partial FT frame
    frameType = FT_NONE;
    ftFrameInLen = 0;
}

/**
 * The main processing loop.
 */
__attribute__((optimize("O3"))) void loop()
{
    if (knxRxTimeout.expired())
    {
        digitalWrite(LED_KNX_RX, false);
    }

    if (knxSerialTimeout.expired())
    {
        digitalWrite(LED_SERIAL_RX, false);
    }

	if (bcu.bus->telegramReceived())
    {
	    digitalWrite(LED_KNX_RX, true);
	    knxRxTimeout.start(LED_KNX_RX_BLINKTIME);
	    processTelegram();
        bcu.bus->discardReceivedTelegram();
    }

	// handle incoming data from the serial line
	if (serial.available())
	{
        digitalWrite(LED_SERIAL_RX, true);
        knxSerialTimeout.start(LED_SERIAL_RX_BLINKTIME);
	}

	while (serial.available())
	{
	    uint8_t byte = serial.read();
		lastSerialRecvTime = millis();

		// start byte / frame detection, fixed or variable frame or just a ack
		if (frameType == FT_NONE)
		{
		    if (byte == FT_ACK)
		    {
		        sendFrameCountBit = !sendFrameCountBit;
		        continue;
		    }
		    else if (byte == FT_FIXED_START)
		        frameType = FT_FIXED_START;
		    else if (byte == FT_VARIABLE_START)
                frameType = FT_VARIABLE_START;
		}

		// buffer overflow prevention
        if (ftFrameInLen < FT_FRAME_SIZE)
            ftFrameIn[ftFrameInLen++] = byte;

        if (byte != FT_END)
        {
            continue;
        }

        if (frameType == FT_FIXED_START)
        {
            if (!isValidFixedFrameHeader(&ftFrameIn[0], ftFrameInLen))
            {
                continue;
            }

            if (!processFixedFrame(&ftFrameIn[0]))
            {
                debugFatal();
            }
            timeoutSerial();
        }
        else if (frameType == FT_VARIABLE_START)
        {
            if (!isValidVariableFrameHeader(&ftFrameIn[0], ftFrameInLen))
            {
                continue;
            }

            if (!processVariableFrame(&ftFrameIn[0], ftFrameInLen))
            {
                debugFatal();
            }
            timeoutSerial();
        }
	}

	if (frameType != FT_NONE && elapsed(lastSerialRecvTime) > FT_TIMEOUT_MS)
	{
	    timeoutSerial();
	}
}
