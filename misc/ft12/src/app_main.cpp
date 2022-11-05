/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include <sblib/io_pin_names.h>
#include <sblib/timeout.h>
#include <sblib/eib/sblib_default_objects.h>
#include <sblib/eib/apci.h>
#include <sblib/serial.h>

#define LED_SERIAL_RX               (PIO0_6) //!< Serial-Rx LED Pin
#define LED_KNX_RX                  (PIO0_7) //!< KNX-Rx LED Pin

#define PIN_FT_SERIAL_TX            (PIN_TX) //!< Serial-Tx Pin
#define PIN_FT_SERIAL_RX            (PIN_RX) //!< Serial-Rx Pin

#define LED_SERIAL_RX_BLINKTIME     (500)    //!< Receiving serial data blinking timeout in milliseconds
#define LED_KNX_RX_BLINKTIME        (500)    //!< Receiving KNX packets blinking timeout in milliseconds

#define FT_OWN_KNX_ADDRESS          (0x11fe) //!< Our own knx-address: 1.1.254
#define FT_FRAME_SIZE               (32)     //!< Maximum size of FT1.2 frames
#define FT_MAX_SEND_RETRY           (1)      //!< Do not repeat sending


byte ftFrameIn[FT_FRAME_SIZE];  //!< Buffer for incoming FT1.2 frames
byte ftFrameOut[FT_FRAME_SIZE]; //!< Buffer for outgoing FT1.2 frames

byte telegramOut1[SB_TELEGRAM_SIZE]; //!< 1.Buffer for outgoing telegrams
byte telegramOut2[SB_TELEGRAM_SIZE]; //!< 2.Buffer for outgoing telegrams

int telegramOutId; //!< Index of the next buffer for an outgoing telegram

int ftFrameInLen;       //!< Length of the data in ftFrameIn
int ftFrameOutLen;      //!< Length of the data in ftFrameOut
int ctrlBase = 0xf0;    //!<

Timeout knxRxTimeout;       //!< KNX-Rx LED blinking timeout
Timeout knxSerialTimeout;   //!< Serial-Rx LED blinking timeout

/**
 * FT frame type
 */
enum FtFrameType
{
    FT_NONE = 0,         //!< none / unspecified
    FT_FIXED = 0x10,     //!< a frame with fixed length
    FT_VARIABLE = 0x68,  //!< a frame with variable length
    FT_END = 0x16,       //!< end of a frame
    FT_ACK = 0xe5,       //!< confirmation
} frameType;

/**
 * FT function code
 */
enum FtFuncCode
{
    FC_SEND_RESET = 0x00,  //!< Reset the remote link
    FC_SEND_UDAT  = 0x03,  //!< User data
    FC_REQ_STATUS = 0x09,  //!< Status
    FC_RESET      = 0x40,  //!< Reset
};

/**
 * External message interface codes
 */
enum EmiCode
{
    PEI_Identify_req = 0xa7, //!< PEI identify request
    PEI_Identify_con = 0xa8, //!< PEI identify reply
    PEI_Switch_req = 0xa9,   //!< Switch the PEI mode

    // Link layer
    L_Data_req      = 0x11,     //!< Send data to the bus
    L_Data_ind      = 0x29,     //!< Data from the bus
    L_Data_conf     = 0x2E,
    L_Busmon_ind    = 0x2B,
    L_Polldata_req  = 0x13,
    L_Polldata_conf = 0x25,

    // Transport layer
    T_Connect_req = 0x43,    //!< Connect request
    T_Data_Connected_req = 0x41, //!< Connected data request
    T_Data_Connected_con = 0x8e, //!< Connected data reply
    T_Connect_con = 0x86,    //!< Connect reply
};

/**
 * Reset the buffers.
 */
void reset()
{
    serial.clearBuffers();

    ftFrameInLen = 0;
    ftFrameOutLen = 0;
    frameType = FT_NONE;
}

/**
 * Initialize the application.
 */
void setup()
{
    bcu.begin(2, 1, 1);         // ABB, dummy something device
    bcu.setOwnAddress(FT_OWN_KNX_ADDRESS);
    bus.maxSendTries(FT_MAX_SEND_RETRY);

    // Disable telegram processing by the lib
    if (userRam.status & BCU_STATUS_TL)
    {
        userRam.status ^= BCU_STATUS_TL | BCU_STATUS_PARITY;
    }

    pinMode(LED_KNX_RX, OUTPUT);	// KNX-Rx LED
    pinMode(LED_SERIAL_RX, OUTPUT);	// Serial-Rx LED

    serial.setTxPin(PIN_FT_SERIAL_TX);
    serial.setRxPin(PIN_FT_SERIAL_RX);
    serial.begin(19200, SERIAL_8E1);
    reset();
    telegramOutId = 0;
}

/**
 * Send a FT frame of variable length. The frame buffer must have enough space
 * so that the checksum and end byte are added.
 *
 * @param frame     The buffer that contains the frame
 * @param funcCode  The function code, e.g. FC_SEND_UDAT
 * @param len       The length of the frame's payload
 */
void sendVariableFrame(byte* frame, int funcCode, int len)
{
    ctrlBase ^= 0x20;

    frame[0] = frame[3] = FT_VARIABLE;
    frame[1] = frame[2] = len;
    frame[4] = ctrlBase | funcCode;

    int checksum = 0;
    for (int i = 0; i <= len; ++i)
        checksum += frame[i + 4];

    frame[len + 5] = checksum;
    frame[len + 6] = FT_END;

    serial.write(frame, len + 7);
}

/**
 * Process a fixed length FT frame
 */
void processFixedFrame()
{
    if (ftFrameIn[1] != ftFrameIn[2])
        return; // discard the frame

    int cmd = ftFrameIn[1];
    if (cmd == FC_RESET)
        reset();

    serial.write(FT_ACK);
}

/**
 * Process a L_DataConnected request in ftFrameIn[]
 */
void processDataConnectedRequest()
{
    ftFrameOut[5]  = T_Data_Connected_con;

    for (int i = 6; i < 10; ++i)
        ftFrameOut[i] = 0;

    int apci = (ftFrameIn[12] << 8) | ftFrameIn[13];
    switch (apci)
    {
    case APCI_DEVICEDESCRIPTOR_READ_PDU:
        int version = bcu.maskVersion();
        serial.write(FT_ACK);
        ftFrameOut[11] = 0x63; // DRL 3 bytes
        ftFrameOut[12] = APCI_DEVICEDESCRIPTOR_RESPONSE_PDU >> 8;
        ftFrameOut[13] = (byte) APCI_DEVICEDESCRIPTOR_RESPONSE_PDU;
        ftFrameOut[14] = version >> 8;
        ftFrameOut[15] = version;
        sendVariableFrame(ftFrameOut, FC_SEND_UDAT, 12);
        break;
    }
}

/**
 * Process a variable length FT frame
 */
void processVariableFrame()
{
    byte* telegramOut;

    int len = ftFrameIn[1];
    if (ftFrameIn[1] != ftFrameIn[2] || ftFrameIn[3] != 0x68)
        return; // discard the frame

    int funcCode = ftFrameIn[4] & 0x0f;
    if (funcCode == FC_SEND_UDAT)
    {
        switch (ftFrameIn[5])  // EMI code
        {
        case PEI_Identify_req:
            serial.write(FT_ACK);
            ftFrameOut[5]  = PEI_Identify_con;
            ftFrameOut[6]  = bus.ownAddress() >> 8;
            ftFrameOut[7]  = bus.ownAddress();
            ftFrameOut[8]  = 0x00;
            ftFrameOut[9]  = 0x01;
            ftFrameOut[10] = 0x00;
            ftFrameOut[11] = 0x01;
            ftFrameOut[12] = 0xE4;
            ftFrameOut[13] = 0x5A;
            ftFrameOut[14] = 0;
            sendVariableFrame(ftFrameOut, FC_SEND_UDAT, 10);
            break;

        case PEI_Switch_req:
            serial.write(FT_ACK);
            ///\ TODO
            break;

        case T_Connect_req:
            serial.write(FT_ACK);
            ftFrameOut[5]  = T_Connect_con;
            ftFrameOut[6]  = 0;
            ftFrameOut[7]  = ftFrameIn[9];
            ftFrameOut[8]  = ftFrameIn[10];
            ftFrameOut[9]  = 0;
            ftFrameOut[10] = 0;
            ftFrameOut[11] = 0;
            sendVariableFrame(ftFrameOut, FC_SEND_UDAT, 7);
            break;

        case T_Data_Connected_req:
            processDataConnectedRequest();
            break;

        case L_Data_req:
            if (telegramOutId)
                telegramOut = telegramOut1;
            else telegramOut = telegramOut2;
            telegramOutId = !telegramOutId;

            for (int i = 3; i < len - 2; ++i)
                telegramOut[i] = ftFrameIn[i + 6];

            telegramOut[0] = 0xB0 | (ftFrameIn[6] & 0x0F);

            bus.sendTelegram(telegramOut, len - 2);


            serial.write(FT_ACK);
            ///\todo WIP L_Data_conf (0x2E) must be send to user, check how it must be done
            ftFrameOut[5]  = L_Data_conf;
            ftFrameOut[6]  = 0;
            ftFrameOut[7]  = ftFrameIn[9];
            ftFrameOut[8]  = ftFrameIn[10];
            ftFrameOut[9]  = 0;
            ftFrameOut[10] = 0;
            ftFrameOut[11] = 0;
            sendVariableFrame(ftFrameOut, FC_SEND_UDAT, 7);
            break;
        }
    }
}

/**
 * Send a telegram of the bus as variable frame.
 */
void processTelegram()
{
    ctrlBase ^= 0x20;

    ftFrameOut[0] = ftFrameOut[3] = FT_VARIABLE;
    ftFrameOut[1] = ftFrameOut[2] = bus.telegramLen + 1;
    ftFrameOut[4] = ctrlBase | FC_SEND_UDAT;
    ftFrameOut[5] = L_Data_ind;

    int checksum = ftFrameOut[4] + ftFrameOut[5];
    for (int i = 0; i < bus.telegramLen - 1; ++i)
    {
        ftFrameOut[i + 6] = bus.telegram[i];
        checksum += bus.telegram[i];
    }

    ftFrameOut[bus.telegramLen + 5] = checksum;
    ftFrameOut[bus.telegramLen + 6] = FT_END;

    serial.write(ftFrameOut, bus.telegramLen + 7);
}

/**
 * The main processing loop.
 */
void loop()
{
    static unsigned int lastSerialRecvTime = 0;

    if (knxRxTimeout.expired())
    {
        digitalWrite(LED_KNX_RX, true);
    }

    if (knxSerialTimeout.expired())
    {
        digitalWrite(LED_SERIAL_RX, true);
    }

	if (bus.telegramReceived())
    {
	    digitalWrite(LED_KNX_RX, false);
	    knxRxTimeout.start(LED_KNX_RX_BLINKTIME);
	    processTelegram();
        bus.discardReceivedTelegram();
    }

	// handle incoming data form the serial line
	if (serial.available())
	{
        digitalWrite(LED_SERIAL_RX, false);
        knxSerialTimeout.start(LED_SERIAL_RX_BLINKTIME);
	}

	while (serial.available())
	{
	    lastSerialRecvTime = millis();
		int byte = serial.read();

		if (frameType == FT_NONE)
		{
		    if (byte == FT_ACK)
		    {
		    }
		    else if (byte == FT_FIXED)
		        frameType = FT_FIXED;
		    else if (byte == FT_VARIABLE)
                frameType = FT_VARIABLE;
		}

        if (ftFrameInLen < FT_FRAME_SIZE)
            ftFrameIn[ftFrameInLen++] = byte;

        if (byte == FT_END)
        {
            if (frameType == FT_FIXED)
            {
                if (ftFrameInLen == 4)
                {
                    processFixedFrame();

                    frameType = FT_NONE;
                    ftFrameInLen = 0;
                }
            }
            else if (frameType == FT_VARIABLE)
            {
                if (ftFrameInLen > 7 && ftFrameIn[1] == ftFrameInLen - 6)
                {
                    processVariableFrame();

                    frameType = FT_NONE;
                    ftFrameInLen = 0;
                }
            }
        }
	}

	if (frameType != FT_NONE && elapsed(lastSerialRecvTime) > 50)
	{
	    // Timeout, discard any received partial FT frame
	    frameType = FT_NONE;
	    ftFrameInLen = 0;
	}
}
