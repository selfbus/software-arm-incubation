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
#include <sblib/bits.h>
#include <sblib/version.h>
#include <cstring>
#include "bcu_ft12.h"
#include "ft12_protocol.h"

#define LED_SERIAL_RX               (PIO0_6) //!< Serial-Rx LED Pin
#define LED_KNX_RX                  (PIO0_7) //!< KNX-Rx LED Pin
#define LED_ON                      (false)  //!< Led is turned on by setting the IO-pin to low/ground
#define LED_OFF                     (true)   //!< Led is turned off by setting the IO-pin to high/+3V3
#define LED_TEST_MS                 (250)    //!< Time in milliseconds every led should light up on startup test

#define PIN_FT_SERIAL_TX            (PIN_TX) //!< Serial-Tx Pin
#define PIN_FT_SERIAL_RX            (PIN_RX) //!< Serial-Rx Pin

#define LED_KNX_RX_BLINKTIME        (100)    //!< Receiving KNX packets blinking timeout in milliseconds

#define FT_OWN_KNX_ADDRESS          (0x11fe) //!< Our own knx-address: 1.1.254
#define FT_FRAME_SIZE               (32)     //!< Maximum size of FT1.2 frames
#define FT_MAX_SEND_RETRY           (0)      //!< Do not repeat sending
#define FT_BAUDRATE                 (19200)  //!< Ft12 baudrate
#define FT_OUTBUFFER_COUNT          (2)      //!< Number of outgoing FT12 frame buffers
APP_VERSION("SBft12  ", "0", "01")

BcuFt12 bcu = BcuFt12();  //!< Bus coupling unit Maskversion 0x0012 of the ft12 module

/** ft12 bit timeout converted in milliseconds */
uint32_t ft12ExchangeTimeoutMs = 2 * ((FT12_EXCHANGE_TIMEOUT_BITS * 1000/FT_BAUDRATE) + 1);
/** ft12 line idle timeout converted in milliseconds */
uint32_t ft12LineIdleTimeoutMs = 2 * ((FT12_LINE_IDLE_TIMEOUT_BITS * 1000/FT_BAUDRATE) + 1);

byte ftFrameIn[FT_FRAME_SIZE];        //!< Buffer for incoming FT1.2 frames
uint8_t ftFrameInLen;                 //!< Length of the data in ftFrameIn
byte ftFrameOut[FT_FRAME_SIZE];       //!< Buffer for preparing FT1.2 frames to send to serial port
byte ftFrameOutBuffer[FT_FRAME_SIZE][FT_OUTBUFFER_COUNT]; //!< Buffer for outgoing FT1.2 frames which are waiting for an ACK
uint8_t ftFrameOutBufferLength[FT_OUTBUFFER_COUNT];       //!< Length of the data in ftFrameOutBuffer

byte repeatCounter;             //! Decrement on every repeat until its zero, initialized with @ref FT12_REPEAT_LIMIT

uint32_t lastSerialRecvTime;
uint32_t lastSerialSendTime;

byte* telegramOut1; //!< 1.Buffer for outgoing telegrams
byte* telegramOut2; //!< 2.Buffer for outgoing telegrams

int telegramOutId; //!< Index of the next buffer for an outgoing knx telegram




bool sendFrameCountBit;
bool rcvFrameCountBit;
int16_t lastChecksum;

Timeout ft12AckTimeout;     //!< waiting for ft12 ACK timeout
Timeout knxRxTimeout;       //!< KNX-Rx LED blinking timeout

FtFrameType frameType = FT_NONE;


void debugFatal()
{
#ifdef DEBUG
    bcu.setProgPin(LED_KNX_RX);
    fatalError();
#endif
}

/**
 * Sends a @ref FT_ACK
 */
void sendft12Ack()
{
    serial.write(FT_ACK);
    serial.flush();
    digitalWrite(LED_SERIAL_RX, LED_OFF);
}

/**
 * Sends a ft12 frame and starts the ack-timeout timer
 * @param frame     ft12 frame to send
 * @param frameSize size of the frame
 */
void sendft12withAckWaiting(byte* frame, int32_t frameSize)
{
    uint8_t sendSize = 15;
    digitalWrite(LED_SERIAL_RX, LED_ON);
    ftFrameOutBufferLength[0] = frameSize;
    memcpy(ftFrameOutBuffer, frame, ftFrameOutBufferLength[0]);
    repeatCounter = FT12_REPEAT_LIMIT;
    uint8_t i = 0;
    while (frameSize >= sendSize)
    {
        serial.write(&frame[i], sendSize);
        i += sendSize;
        frameSize -= sendSize;
    }
    serial.write(&frame[i], frameSize);

    lastSerialSendTime = millis();
    ft12AckTimeout.start(ft12ExchangeTimeoutMs);
}

void sendft12RepeatedFrame()
{
    return; ///\todo repeating doesn't work reliably right now

    if ((ftFrameOutBufferLength[0] == 0) || (repeatCounter <= 0))
    {
        ftFrameOutBufferLength[0] = 0;
        ft12AckTimeout.stop();
        return;
    }

    repeatCounter--;
    digitalWrite(LED_SERIAL_RX, LED_ON);
    serial.write(ftFrameOutBuffer[0], ftFrameOutBufferLength[0]);
    ft12AckTimeout.start(ft12ExchangeTimeoutMs);
}

/**
 * Send a FT frame of fixed length
 *
 * @param frame          The buffer that contains the frame
 * @param funcCode       The function code, e.g. FC_RESET
 * @param frameCountBit  Frame count bit of the control byte
 */
void sendFixedFrame(byte* frame, const FtFunctionCode& funcCode, const bool& frameCountBit)
{
    frame[0] = FT_FIXED_START;
    frame[1] = 0xD0 | (funcCode & 0x0f);
    if (frameCountBit)
    {
        frame[1] |= 0x20;
    }
    frame[2] = frame[1];
    frame[3] = FT_END;
    sendft12withAckWaiting(frame, FIXED_FRAME_LENGTH);
}

/**
 * Reset the buffers.
 */
void reset()
{
    serial.clearBuffers();
    sendFrameCountBit = true;
    lastChecksum = -1;
    ftFrameInLen = 0;
    for (uint8_t i = 0; i < FT_OUTBUFFER_COUNT; i++)
    {
        ftFrameOutBufferLength[i] = 0;
    }
    ft12AckTimeout.stop();
    frameType = FT_NONE;
    telegramOutId = 0;
    lastSerialRecvTime = 0;
    lastSerialSendTime = 0;
}

/**
 * Initialize the application.
 */
BcuBase* setup()
{
    telegramOut1 = new byte(bcu.maxTelegramSize());
    telegramOut2 = new byte(bcu.maxTelegramSize());

    // led init and test
    pinMode(LED_KNX_RX, OUTPUT);    // KNX-Rx LED
    digitalWrite(LED_KNX_RX, LED_ON);
    delay(LED_TEST_MS);
    pinMode(LED_SERIAL_RX, OUTPUT); // Serial-Rx LED
    digitalWrite(LED_SERIAL_RX, LED_ON);
    delay(LED_TEST_MS);
    digitalWrite(LED_KNX_RX, LED_OFF);
    delay(LED_TEST_MS);
    digitalWrite(LED_SERIAL_RX, LED_OFF);

    bcu.begin();
    bcu.setOwnAddress(FT_OWN_KNX_ADDRESS);
    bcu.bus->maxSendRetries(FT_MAX_SEND_RETRY);
    bcu.bus->maxSendBusyRetries(FT_MAX_SEND_RETRY);
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
    return (&bcu);
}


/**
 * This is a VERY VERY dirty hack to replace the KNX sender address 0.0.0 with 15.15.255 for @ref APCI_INDIVIDUAL_ADDRESS_RESPONSE_PDU
 * Older ARM Selfbus devices with clean flash have the default KNX-address 0.0.0 which is not allowed in KNX specification so e.g. knxd ignores their response.
 * Even KNX-address programming with a broadcasted @ref APCI_INDIVIDUAL_ADDRESS_READ_PDU will fail.
 *
 * @warning This is a VERY VERY dirty hack and not even close to KNX spec!
 *
 * @param frame          The buffer that contains the frame
 * @param funcCode       The function code, e.g. FC_SEND_UDAT
 * @param emi            The @ref EmiCode to send
 * @param userDataLength The length of the frame's payload
 *
 * @return true if sender address was replaced with 15.15.255, otherwise false
 */
bool dirtyCheckAndReplaceInvalidDefaultSenderAddress(byte* frame, const FtFunctionCode& funcCode, const EmiCode& emi, const uint8_t& userDataLength)
{
    // invalid frame example:
    // # 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    //  68 0A 0A 68 D3 29 B0 00 00 00 00 E1 01 40 CE 16

    // user data indication with length 10?
    if ((emi != L_Data_Ind) || (userDataLength != 0x0A) || ((funcCode & 0x0f) != FC_SEND_UDAT))
    {
        return (false);
    }

    // check 0.0.0 for KNX sender and destination address
    if ((frame[7] != 0) || (frame[8] != 0) || (frame[9] != 0) || (frame[10] != 0))
    {
        return (false);
    }

    // is apci command = APCI_INDIVIDUAL_ADDRESS_RESPONSE_PDU ??
    if (makeWord(frame[12], frame[13]) != APCI_INDIVIDUAL_ADDRESS_RESPONSE_PDU)
    {
        return (false);
    }

    // is response length = 1??
    if ((frame[11] & 0x0f) != 1) // response length in low nibble
    {
        return (false);
    }

    // okay seems like we had a bad day
    // -> inject sender address 15.15.255
    frame[7] = 0xff;
    frame[8] = 0xff;
    return (true);
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
    // This is a VERY VERY dirty hack for older Selfbus devices with default knx address 0.0.0
    dirtyCheckAndReplaceInvalidDefaultSenderAddress(frame, funcCode, emi, userDataLength);

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

    frame[4 + userDataLength] = calcCheckSum(frame, userDataLength);
    frame[5 + userDataLength] = FT_END;
    sendft12withAckWaiting(frame, userDataLength + VARIABLE_FRAME_HEADER_LENGTH);
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
            sendft12Ack();
            reset();
            return true;
            break;
        case FC_REQ_STATUS:
            sendft12Ack();
            return true; ///\todo FC_REQ_STATUS
            break;
        default:
            return false;
    }
    return true;
}

/**
 * Process a L_DataConnected request in ftFrameIn[]
 */
void processDataConnectedRequest()
{
    for (uint32_t i = VARIABLE_FRAME_HEADER_LENGTH; i < 10; ++i)
    {
        ftFrameOut[i] = 0;
    }

    uint16_t apci = makeWord(ftFrameIn[12], ftFrameIn[13]);
    switch (apci)
    {
    case APCI_DEVICEDESCRIPTOR_READ_PDU:
        sendft12Ack();
        uint16_t version = 0x0012;
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
    case PEI_Identify_Req: // KNX Spec. 3/6/3 3.3.9.5 p.54
        sendft12Ack();
        ftFrameOut[6]  = HIGH_BYTE(bcu.ownAddress()); // create PEI_Identify_con
        ftFrameOut[7]  = lowByte(bcu.ownAddress());
        ftFrameOut[8]  = 0x00; // 6 bytes KNX serial number
        ftFrameOut[9]  = 0x01;
        ftFrameOut[10] = 0x00;
        ftFrameOut[11] = 0x01;
        ftFrameOut[12] = 0xE4;
        ftFrameOut[13] = 0x5A;
        ftFrameOut[14] = 0;
        sendVariableFrame(ftFrameOut, FC_SEND_UDAT, PEI_Identify_Con, 10, sendFrameCountBit);
        break;

    case PEI_Switch_Req: // KNX Spec. 3/6/3 3.1.4 p.14
        reset();
        sendft12Ack();
        break;

    case T_Connect_Req:
        sendft12Ack();
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
        sendft12Ack();
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
        ftFrameOut[i + VARIABLE_FRAME_HEADER_LENGTH] = bcu.bus->telegram[i];
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
//__attribute__((optimize("O3"))) void loop()
void loop()
{
    if (knxRxTimeout.expired())
    {
        digitalWrite(LED_KNX_RX, LED_OFF);
    }

	int32_t byte;
	while ((byte = serial.read()) > -1)
	{
		lastSerialRecvTime = millis();
		// start byte / frame detection, fixed or variable frame or just a ack
		if (frameType == FT_NONE)
		{
		    switch (byte)
		    {
		        case FT_ACK:
                {
                    ft12AckTimeout.stop();
                    ftFrameOutBufferLength[0] = 0;
                    sendFrameCountBit = !sendFrameCountBit;
                    digitalWrite(LED_SERIAL_RX, LED_OFF);
                    continue;
                }
		        case FT_FIXED_START:
                    frameType = FT_FIXED_START;
                    break;
		        case FT_VARIABLE_START:
                    frameType = FT_VARIABLE_START;
                    break;
		        case 0xA0:
		            reset();
		            continue;
		            break;
		        default:
		            frameType = FT_NONE; // we should never land here, otherwise something is really wrong
		            // sendFixedFrame(ftFrameOut, FC_SEND_RESET, sendFrameCountBit);
		            continue;
		            // debugFatal();
		    }
		}

		// buffer overflow prevention
        if (ftFrameInLen >= FT_FRAME_SIZE)
        {
            reset();
            continue;
        }

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

    if (ft12AckTimeout.expired())
    {
        sendft12RepeatedFrame();
    }

    if (bcu.bus->telegramReceived())
    {
        digitalWrite(LED_KNX_RX, LED_ON);
        knxRxTimeout.start(LED_KNX_RX_BLINKTIME);
        if (ftFrameOutBufferLength[0] == 0)
        {
            processTelegram();
            bcu.bus->discardReceivedTelegram();
        }
    }

	if (frameType != FT_NONE && elapsed(lastSerialRecvTime) > ft12ExchangeTimeoutMs)
	{
	    timeoutSerial();
	}
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{
    loop();
}
