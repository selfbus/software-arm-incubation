/*
 *  bus.cpp - Low level EIB bus access.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bus.h>

#include <sblib/core.h>
#include <sblib/platform.h>
#include <sblib/eib/addr_tables.h>
#include <sblib/eib/user_memory.h>


// The EIB bus access object
Bus bus(timer16_1, PIO1_8, PIO1_10, CAP0, MAT1);

// The interrupt handler for the EIB bus access object
BUS_TIMER_INTERRUPT_HANDLER(TIMER16_1_IRQHandler, bus);


/*
 * The timer16_1 is used as follows:
 *
 * Capture register CR0 is used for receiving
 * Match register MR0 or MR1 is used as PWM for sending, depending on which output pin is used
 * Match register MR3 is used for timeouts while sending / receiving
 *
 */

// Enable debug statements for debugging the bus access in this file
#define D(x) x
//#define D(x)


#define LPC_TMR16B1_MR_OUT LPC_TMR16B1->MR1    /* Timer match register for bus out */
#define LPC_IOCON_BUS_OUT LPC_IOCON->PIO1_10   /* IOCON register for bus out */
#define BUS_OUT_IOCON_PWM 2                    /* IOCON for bus out: PWM channel selection */

#define BUS_OUT_PORT_PIN 1,10                  /* Port and pin for bus out */
int txPin = PIO1_10;





// Telegram repeat flag in byte #0 of the telegram: 1=not repeated, 0=repeated
#define SB_TEL_REPEAT_FLAG 0x20

static int debugLine = 0;


static LPC_GPIO_TypeDef (* const LPC_GPIO[4]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3 };


// Mask for the timer flag of the capture channel
#define CAPTURE_FLAG (8 << captureChannel)

// Default time between two bits (104 usec)
#define BIT_TIME 104

// Time between two bits (69 usec)
#define BIT_WAIT_TIME 69

// Maximum time from start bit to stop bit, including a safety extra: BIT_TIME*10 + BIT_TIME/2
#define BYTE_TIME 1090

// Time to wait before sending an ACK: BIT_TIME * 11 + (BIT_TIME / 4)
#define SEND_ACK_WAIT_TIME 1170

// Time to wait before starting to send: BIT_TIME * 50
#define SEND_WAIT_TIME 5200

// The value for the prescaler
#define TIMER_PRESCALER (SystemCoreClock / 1000000 - 1)


Bus::Bus(Timer& aTimer, int aRxPin, int aTxPin, int aCaptureChannel, int aPwmChannel)
:timer(aTimer)
,rxPin(aRxPin)
,txPin(aTxPin)
,captureChannel(aCaptureChannel)
,pwmChannel(aPwmChannel)
{
    timeChannel = (pwmChannel + 2) & 3;  // +2 to be compatible to old code during refactoring
    state = Bus::IDLE;
}

void Bus::begin()
{
    ownAddr = (userEeprom.addrTab[0] << 8) | userEeprom.addrTab[1];

    telegramLen = 0;

    state = Bus::IDLE;
    sendAck = 0;
    sendCurTelegram = 0;
    sendNextTel = 0;

    //LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 8; // Enable the clock for the timer
    timer.begin();

    //
    // Init GPIOs for bus access
    //

    // Configure bus input
    pinMode(rxPin, INPUT_CAPTURE);
    //LPC_IOCON->PIO1_8 = 0x21;   // 0x21: Select timer1_16 CAP0, input, no pull-up/down, hysteresis
                                // 0x29: Select timer1_16 CAP0, input, pull-down, hysteresis
                                // 0x50: Select timer1_16 CAP0, input, pull-up, hysteresis
                                // 0x0a: Select timer1_16 CAP0, input, pull-down, no hysteresis

    // Configure bus output
    pinMode(txPin, OUTPUT_MATCH);
    //LPC_TMR16B1_MR_OUT = 0;     // Set bus-out match to 0 to have always 1
//    LPC_TMR16B1->EMR = 0;       // Clear timer match for bus-out

    //    LPC_TMR16B1->EMR = 0x82;  // Set output to 1 on match for bus-out
//    LPC_IOCON_BUS_OUT = 0x90;   // Set pin to normal output
//    LPC_GPIO[1]->DIR |= 0x400;  // Set bus-out pin to output
//    LPC_TMR16B1->PWMC = 2;      // Enable PWM for bus-out
    timer.pwmEnable(pwmChannel);

    digitalWrite(txPin, 0);

//    LPC_TMR16B1->CCR = 6;       // Capture CR0 on falling edge, with interrupt
    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);
//    LPC_TMR16B1->MCR = 0;       // Do not handle timer matches
//    LPC_TMR16B1->TCR = 1;       // Enable the timer
    timer.start();
//    NVIC_EnableIRQ(TIMER_16_1_IRQn); // Enable the timer interrupt
    timer.interrupts();

    //
    // Calculate timer ticks
    //

    //LPC_TMR16B1->PR = SystemCoreClock / 1000000 - 1;
    timer.prescaler(TIMER_PRESCALER);

//    LPC_TMR16B1->MR3 = 0xffff;
    timer.match(timeChannel, 0xfffe);
    timer.matchMode(timeChannel, RESET);
//    LPC_TMR16B1->MCR = 0;    // Do not interrupt or reset timer on timeout (match of MR3)
    timer.match(pwmChannel, 0xffff);

    //
    // Init GPIOs for debugging
    //
    D(pinMode(PIO3_0, OUTPUT));
    D(pinMode(PIO3_1, OUTPUT));
    D(pinMode(PIO3_2, OUTPUT));
    D(pinMode(PIO3_3, OUTPUT));
    D(pinMode(PIO0_6, OUTPUT));
    D(pinMode(PIO0_7, OUTPUT));
    D(pinMode(PIO2_8, OUTPUT));
    D(pinMode(PIO2_9, OUTPUT));
    D(pinMode(PIO2_10, OUTPUT));

    D(digitalWrite(PIO3_0, 0));
    D(digitalWrite(PIO3_1, 0));
    D(digitalWrite(PIO3_2, 0));
    D(digitalWrite(PIO3_3, 0));
    D(digitalWrite(PIO0_6, 0));
    D(digitalWrite(PIO0_7, 0));
    D(digitalWrite(PIO2_8, 0));
    D(digitalWrite(PIO2_9, 0));
    D(digitalWrite(PIO2_10, 0));
}

void Bus::idleState()
{
    //    LPC_TMR16B1->CCR = 6;     // Capture CR0 on falling edge, with interrupt
        timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);

//    LPC_TMR16B1->MR3 = 0xffff;
//    LPC_TMR16B1->MCR = 0;     // Do not interrupt or reset timer on timeout (match of MR3)
    timer.matchMode(timeChannel, RESET);
    timer.match(timeChannel, 0xfffe);
    timer.match(pwmChannel, 0xffff);

    state = Bus::IDLE;
    sendAck = 0;

//    digitalWrite(txPin, 0); // Set bus-out pin to 0
    //GPIOSetValue(BUS_OUT_PORT_PIN, 0);

//    LPC_IOCON_BUS_OUT &= ~(LPC_IOCON_BUS_OUT | BUS_OUT_IOCON_PWM); // Disable bus-out output
//    pinMode(txPin, INPUT);
}

void Bus::handleTelegram(bool valid)
{
    D(digitalWrite(PIO3_3, 1));         // purple: end of telegram

    if (nextByteIndex >= 8 && valid) // Received a valid telegram with correct checksum
    {
        int destAddr = (telegram[3] << 8) | telegram[4];
        bool processTel = false;

        // We ACK the telegram only if it's for us
        if (telegram[5] & 0x80)
        {
            if (destAddr == 0 || indexOfAddr(destAddr) >= 0)
                processTel = true;
        }
        else if (destAddr == ownAddr)
        {
            processTel = true;
        }

        // Only process the telegram if it is for us or if we want to get all telegrams
        if (!(userRam.status & BCU_STATUS_TL))
        {
            telegramLen = nextByteIndex;
        }
        else if (processTel)
        {
            telegramLen = nextByteIndex;
            sendAck = SB_BUS_ACK;
        }
    }
    else if (nextByteIndex == 1)   // Received a spike or a bus acknowledgment
    {
        currentByte &= 0xff;

        if ((currentByte == SB_BUS_ACK || sendTries >= 3) && sendCurTelegram)
        {
            // Prepare the next telegram for sending
            sendCurTelegram[0] = 0;
            sendCurTelegram = sendNextTel;
            sendNextTel = 0;
            sendTries = 0;
            sendTelegramLen = 0;
        }
    }
    else // Received more than one byte, but too short for a telegram or wrong checksum
    {
        telegramLen = 0;
        sendAck = SB_BUS_NACK;
    }

//    LPC_TMR16B1->MCR = 0x600;  // Interrupt and reset timer on timeout (match of MR3)
    timer.matchMode(timeChannel, INTERRUPT | RESET);
    state = Bus::SEND_INIT;    // might be changed by sb_idle() below
    debugLine = __LINE__;

    if (sendAck)
    {
        // Wait before sending the bus acknowledgement
//        LPC_TMR16B1->MR3 = BIT_TIME * 11 + (BIT_TIME >> 1);
        timer.match(timeChannel, SEND_ACK_WAIT_TIME);
    }
    else if (sendCurTelegram)
    {
        // Wait at least 50 bit times before sending
//        LPC_TMR16B1->MR3 = SEND_WAIT_TIME;
        timer.match(timeChannel, SEND_WAIT_TIME);
    }
    else idleState();
}

void Bus::timerInterruptHandler()
{
    D(static unsigned short tick = 0);
    unsigned short rxTime = LPC_TMR16B1->CR0;
//    unsigned int val;

    // Debug output
    D(digitalWrite(PIO0_6, ++tick & 1));  // brown: interrupt tick
    D(digitalWrite(PIO3_0, state==Bus::SEND_BIT_0)); // red
    D(digitalWrite(PIO3_1, 0));           // orange
    D(digitalWrite(PIO3_2, 0));               // yellow: end of byte
    D(digitalWrite(PIO3_3, 0));              // purple: end of telegram
    D(digitalWrite(PIO2_8, 0));              //
    D(digitalWrite(PIO2_9, 0));              //

STATE_LOOP:
    switch (state)
    {
    case Bus::IDLE:
        if (timer.flags() & CAPTURE_FLAG) // Timeout: do nothing
            break;
        sendAck = 0;
        nextByteIndex = 0;
        checksum = 0xff;
        valid = 1;
        // no break here

    case Bus::RECV_START:
        D(digitalWrite(PIO3_1, 1));   // orange
        if (timer.flags() & CAPTURE_FLAG) // Timeout while waiting for the next start byte
        {
            handleTelegram(valid && !checksum);
            break;
        }
//        LPC_TMR16B1->MR3 = BYTE_TIME;
        timer.match(timeChannel, BYTE_TIME);
//        LPC_TMR16B1->TCR = 2;       // Reset the timer
        timer.reset();

//        LPC_TMR16B1->TCR = 1;       // Enable the timer
        timer.start();
//        LPC_TMR16B1->MCR = 0x600;   // Interrupt and reset timer on timeout (match of MR3)
        timer.matchMode(timeChannel, INTERRUPT | RESET);
        state = Bus::RECV_BYTE;
        currentByte = 0;
        bitTime = 0;
        bitMask = 1;
        parity = 1;
        break;

    case Bus::RECV_BYTE:
        if (timer.flags() & CAPTURE_FLAG)
            rxTime = BYTE_TIME;

        if (rxTime >= bitTime + BIT_WAIT_TIME)
        {
            bitTime += BIT_TIME;
            while (rxTime >= bitTime + BIT_WAIT_TIME && bitMask <= 0x100)
            {
                currentByte |= bitMask;
                parity = !parity;

                bitTime += BIT_TIME;
                bitMask <<= 1;
            }

            bitMask <<= 1;
        }

        if (timer.flags() & CAPTURE_FLAG)  // Timer timeout: end of byte
        {
//            D(digitalWrite(PIO3_2, 1));        // yellow: end of byte
//            D(GPIOSetValue(3, 1, parity)); // orange: parity bit ok

            valid &= parity;
            if (nextByteIndex < SB_TELEGRAM_SIZE)
            {
                telegram[nextByteIndex++] = currentByte;
                checksum ^= currentByte;
            }

            state = Bus::RECV_START;            // wait for the next byte's start bit
//            LPC_TMR16B1->MR3 = BIT_TIME << 2;  // timeout for waiting
            timer.match(timeChannel, BIT_TIME * 4);
        }
        break;

    case Bus::SEND_INIT:
        if (!sendAck && !sendCurTelegram)
        {
            idleState();
            break;
        }

        state = Bus::SEND_START;
        if (sendCurTelegram)
        {
            sendTelegramLen = telegramSize(sendCurTelegram) + 1;
            D(digitalWrite(PIO2_10, 0));             //
        }
        else sendTelegramLen = 0;
        // No break here

    case Bus::SEND_START:
#ifdef DEBUG
        if (!sendAck && !sendCurTelegram)
            fatalError();
#endif
        if (!sendAck)
        {
            if (sendTries == 1)
            {
                // If it is the first repeat, then mark the telegram as being repeated and correct the checksum
                sendCurTelegram[0] &= ~SB_TEL_REPEAT_FLAG;
                sendCurTelegram[sendTelegramLen - 1] ^= SB_TEL_REPEAT_FLAG;
            }

            ++sendTries;
        }

//        LPC_TMR16B1_MR_OUT = BIT_WAIT_TIME;  // Set the output to 1 after the wait time
//        LPC_TMR16B1->MR3 = BIT_TIME;   // Interrupt after bit time
//        val = LPC_IOCON_BUS_OUT | BUS_OUT_IOCON_PWM;

//        if (state != Bus::SEND_START) // do nothing if the bus is busy
        D(digitalWrite(PIO3_2, 1));     // yellow: start of byte
        if (!digitalRead(rxPin)) // do nothing if the bus is busy
        {
            state = IDLE;
            return;
        }

//        LPC_TMR16B1->TC = BIT_WAIT_TIME;// Change the timer to have a 1 almost immediately
        timer.match(pwmChannel, BIT_WAIT_TIME);
        timer.value(BIT_WAIT_TIME);
//        LPC_IOCON_BUS_OUT = val;        // Configure bus-out pin as PWM output
//        pinMode(txPin, OUTPUT_MATCH);
        timer.match(timeChannel, BIT_TIME);
//        LPC_TMR16B1->MCR = 0x600;       // Interrupt and reset timer on match of MR3
        timer.matchMode(timeChannel, INTERRUPT | RESET);
//        LPC_TMR16B1->CCR = 2;           // Capture CR0 on falling edge, without interrupt
        timer.captureMode(captureChannel, FALLING_EDGE);
        D(digitalWrite(PIO3_2, 0));     // yellow: start of byte
        state = Bus::SEND_BIT_0;
        nextByteIndex = 0;
        break;

    case Bus::SEND_BIT_0:
        state = Bus::SEND_BYTE;

        if (sendAck)
            currentByte = sendAck;
        else currentByte = sendCurTelegram[nextByteIndex++];

        // Calculate the parity bit
        for (bitMask = 1; bitMask < 0x100; bitMask <<= 1)
        {
            if (currentByte & bitMask) currentByte ^= 0x100;
        }
        bitMask = 1;
        // no break here

    case Bus::SEND_BYTE:
//        D(digitalWrite(PIO3_2, 1));    // yellow: send next bits
        rxTime = BIT_TIME;
        while ((currentByte & bitMask) && bitMask <= 0x100)
        {
            bitMask <<= 1;
            rxTime += BIT_TIME;
        }
        bitMask <<= 1;

        if (bitMask > 0x200)
        {
            rxTime += BIT_TIME * 3; // Stop bit + inter-byte timeout

            if (nextByteIndex < sendTelegramLen && !sendAck)
            {
                state = Bus::SEND_BIT_0;
            }
            else
            {
                state = Bus::SEND_END;
//                LPC_IOCON_BUS_OUT &= ~(LPC_IOCON_BUS_OUT | BUS_OUT_IOCON_PWM); // Disable bus-out output
//                pinMode(txPin, INPUT);
//                timer.matchMode(timeChannel, INTERRUPT | RESET);
            }
        }

//        LPC_TMR16B1->MR3 = rxTime;   // Reset and interrupt at the next 0 bit
//        LPC_TMR16B1_MR_OUT = rxTime - (BIT_TIME - BIT_WAIT_TIME);
        timer.match(timeChannel, rxTime);

        if (state == Bus::SEND_END)
            timer.match(pwmChannel, 0xffff);
        else timer.match(pwmChannel, rxTime - (BIT_TIME - BIT_WAIT_TIME));

        break;

    case Bus::SEND_END:
        D(digitalWrite(PIO2_9, 1));
//        LPC_TMR16B1->MR3 = SEND_WAIT_TIME;
        timer.match(timeChannel, SEND_WAIT_TIME);
//        LPC_TMR16B1->MCR = 0x600;  // Interrupt and reset timer on timeout (match of MR3)
//        LPC_TMR16B1->CCR = 6;      // Capture CR0 on falling edge, with interrupt
        timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);

        sendAck = 0;
        state = Bus::SEND_WAIT;    // Wait for ACK or resend / send next telegram
        break;

    case Bus::SEND_WAIT:
        if (!(timer.flags() & CAPTURE_FLAG)) // Start receiving if its not a timeout
        {
            state = Bus::IDLE;
            goto STATE_LOOP;
        }

        // FIXME the next telegram is not sent if sendTelegramTries>=3
        if (sendAck || (sendTelegramLen && sendTries < 3))
        {
            D(digitalWrite(PIO2_10, 0));
            state = Bus::SEND_START;
            debugLine = __LINE__;
            goto STATE_LOOP;
        }
        D(digitalWrite(PIO2_8, 1));
        // no break here

    default:
        idleState();
        break;
    }

//    LPC_TMR16B1->IR = 0xff;
    timer.resetFlags();
}

/**
 * Prepare the telegram for sending. Set the sender address to our own
 * address, and calculate the checksum of the telegram.
 * Stores the checksum at telegram[length].
 *
 * @param telegram - the telegram to process
 * @param length - the length of the telegram
 */
void Bus::prepareTelegram(unsigned char* telegram, unsigned short length) const
{
    unsigned char checksum = 0xff;
    unsigned short i;

    // Set the sender address
    telegram[1] = ownAddr >> 8;
    telegram[2] = ownAddr;

    // Calculate the checksum
    for (i = 0; i < length; ++i)
        checksum ^= telegram[i];
    telegram[length] = checksum;
}

/**
 * Send a telegram. The checksum byte will be added at the end of telegram[].
 * Ensure that there is at least one byte space at the end of telegram[].
 *
 * @param telegram - the telegram to be sent.
 * @param length - the length of the telegram in sbSendTelegram[], without the checksum
 */
void Bus::sendTelegram(unsigned char* telegram, unsigned short length)
{
    prepareTelegram(telegram, length);

    // Wait until there is a space in the sending queue
    while (sendNextTel)
    {
    }

    if (!sendCurTelegram) sendCurTelegram = telegram;
    else if (!sendNextTel) sendNextTel = telegram;
    else fatalError();   // soft fault: send buffer overflow

    // Start sending if the bus is idle
    if (state == IDLE) // && !(LPC_TMR16B1->IR & 0x10))
    {
        sendTries = 0;
        state = SEND_INIT;
        debugLine = __LINE__;

//        LPC_TMR16B1->MR3 = 100;
//        LPC_TMR16B1->MCR = 0x600;  // Interrupt and reset timer on match of MR3
//        LPC_TMR16B1->TC = 99;

        timer.match(timeChannel, 1);
        timer.matchMode(timeChannel, INTERRUPT | RESET);
        timer.value(0);
    }
}
