// sb_bus.c

#include "sb_bus.h"
#include "sb_const.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif
#include "gpio.h"

/**
 * Define to use pin 1.9, comment out to use pin 1.10 for bus output
 */
#define SB_BUS_OUT_P1_10

#ifdef SB_BUS_OUT_P1_9
#  define LPC_TMR16B1_MR_OUT LPC_TMR16B1->MR0
#  define LPC_IOCON_BUS_OUT LPC_IOCON->PIO1_9
#  define BUS_OUT_IOCON_PWM 1
#  define BUS_OUT_PORT_PIN 1,9
#else // use P1_10
#  define LPC_TMR16B1_MR_OUT LPC_TMR16B1->MR1
#  define LPC_IOCON_BUS_OUT LPC_IOCON->PIO1_10
#  define BUS_OUT_IOCON_PWM 2
#  define BUS_OUT_PORT_PIN 1,10
#endif


//----- exported variables -----
// The state of the lib's receiver/sender
enum SbState sbState;

// The telegram buffer for receiving
unsigned char sbRecvTelegram[SB_TELEGRAM_SIZE];

// The size of the received telegram in bytes. 0 if no telegram was received.
unsigned short sbRecvTelegramLen;

// The telegram buffer for sending
unsigned char sbSendTelegram[SB_TELEGRAM_SIZE];

// The size of the to be sent telegram in bytes (including the checksum).
unsigned short sbSendTelegramLen;

// Status byte (from user-ram 0x60)
unsigned char sbStatus;

// Our own physical address on the bus
unsigned short sbOwnPhysicalAddr;

//----- private variables -----

// Default time between two bits (104 usec)
static unsigned short sbTimeBit;

// Time between two bits (69 usec)
static unsigned short sbTimeBitWait;

// Duration of a bit pulse (35 usec)
static unsigned short sbTimeBitPulse;

// Maximum time from start bit to stop bit, including a safety extra
static unsigned short sbTimeByte;

// The current byte that is received/sent
static unsigned short sbCurrentByte;

// The number of the next byte in the telegram
static short sbNextByte;

// The number of repeats when sending a telegram
static short sbSendTelegramTries;

// Send an acknowledge or not-acknowledge byte if != 0
static unsigned char sbSendAck;

//----- Telegram bits and constants -----

// Telegram repeat flag in byte #0 of the telegram: 1=not repeated, 0=repeated
#define SB_TEL_REPEAT_FLAG 0x20

/*
 * The timer16_1 is used as follows:
 *
 * Capture register CR0 is used for receiving
 * Match register MR0 or MR1 is used as PWM for sending, depending on which output pin is used
 * Match register MR3 is used for timeouts while sending / receiving
 *
 */

/**
 * Switch to idle state
 */
static void sb_idle()
{
    LPC_TMR16B1->MCR = 0;      // Do not handle timer matches
    LPC_TMR16B1->CCR = 6;      // Capture CR0 on falling edge, with interrupt
    LPC_TMR16B1->MR3 = 0xffff; // Set timer match 3 to maximum value
    sbState = SB_IDLE;

    GPIOSetValue(BUS_OUT_PORT_PIN, 0);       // Set bus-out pin to 0
    LPC_IOCON_BUS_OUT &= ~(LPC_IOCON_BUS_OUT | BUS_OUT_IOCON_PWM); // Disable bus-out output
}

/**
 * Timer16 #1 interrupt handler
 */
void TIMER16_1_IRQHandler()
{
    static unsigned short bitMask;
    static unsigned short tick = 0;
    static unsigned short bitTime = 0;
    static unsigned char parity, checksum;
    unsigned short timer = LPC_TMR16B1->CR0;
    unsigned int val;

    // Debug output
    GPIOSetValue(0, 6, ++tick & 1); 	// brown: interrupt tick
    GPIOSetValue(3, 0, sbState==SB_SEND_BIT_0); // red
    GPIOSetValue(3, 1, 0);          	// orange
    GPIOSetValue(3, 2, 0);				// yellow: end of byte
    GPIOSetValue(3, 3, 0);              // purple: end of telegram

    switch (sbState)
    {
    case SB_IDLE:
        sbSendAck = 0;
        sbNextByte = 0;
        checksum = 0xff;
        // no break here

    case SB_RECV_START:
        GPIOSetValue(3, 1, 1);  // orange
        if (LPC_TMR16B1->IR & 0x08)	// Timeout while waiting for next start byte
        {
            GPIOSetValue(3, 3, 1);              // purple: end of telegram
            if (sbNextByte >= 8 && !checksum) // Received a valid telegram with correct checksum
            {
                sbRecvTelegramLen = sbNextByte;
                sbSendAck = SB_BUS_ACK;
            }
            else if (sbNextByte == 1)   // Received a spike or a bus acknowledgment
            {
                sbSendAck = 0;
                sbCurrentByte &= 0xff;

                if (sbCurrentByte == SB_BUS_ACK || sbSendTelegramTries >= 3) sbSendTelegramLen = 0;
            }
            else // Received more than one byte, but too short for a telegram or wrong checksum
            {
                sbRecvTelegramLen = 0;
                sbSendAck = SB_BUS_NACK;
            }

            if (sbSendAck)
            {
                LPC_TMR16B1->MR3 = sbTimeBit * 15; // Wait 15 bit times before sending the bus acknowledgement
                LPC_TMR16B1->MCR = 0x600;  // Interrupt and reset timer on timeout (match of MR3)
                sbState = SB_SEND_INIT;
            }
            else if (sbSendTelegramLen > 0)
            {
                LPC_TMR16B1->MR3 = sbTimeBit * 50; // Wait at least 50 bit times before sending
                LPC_TMR16B1->MCR = 0x600;  // Interrupt and reset timer on timeout (match of MR3)
                sbState = SB_SEND_INIT;
            }
            else
            {
                sb_idle();
            }
            break;
        }

        LPC_TMR16B1->MR3 = sbTimeByte;
        LPC_TMR16B1->TCR = 2;		// Reset the timer
        LPC_TMR16B1->TCR = 1;		// Enable the timer
        LPC_TMR16B1->MCR = 0x600;	// Interrupt and reset timer on timeout (match of MR3)
        sbState = SB_RECV_BYTE;
        sbCurrentByte = 0;
        bitTime = 0;
        bitMask = 1;
        parity = 1;
        break;

    case SB_RECV_BYTE: 			// DEBUG info: first expected byte is 0xbc (10111100b, parity 1)
        if (LPC_TMR16B1->IR & 0x08) timer = sbTimeByte;

        if (timer >= bitTime + sbTimeBitWait)
        {
            bitTime += sbTimeBit;
            while (timer >= bitTime + sbTimeBitWait && bitMask <= 0x100)
            {
                sbCurrentByte |= bitMask;
                parity = !parity;

                bitTime += sbTimeBit;
                bitMask <<= 1;
            }

            bitMask <<= 1;
        }

        if (LPC_TMR16B1->IR & 0x08)  // Timer timeout: end of byte
        {
            GPIOSetValue(3, 2, 1);		// yellow: end of byte
//            GPIOSetValue(3, 1, parity);	// orange: parity bit ok

            if (sbNextByte < SB_TELEGRAM_SIZE)
            {
                sbRecvTelegram[sbNextByte++] = sbCurrentByte;
                checksum ^= sbCurrentByte;
            }

            sbState = SB_RECV_START;			// wait for the next byte's start bit
            LPC_TMR16B1->MR3 = sbTimeBit << 2;	// timeout for waiting
        }
        break;

    case SB_SEND_INIT:
        if (!(LPC_TMR16B1->IR & 0x08)) // Do nothing if it's not a timeout
            break;

        if (sbSendTelegramTries == 1)
        {
            // If it is the first repeat, then mark the telegram as being repeated and correct the checksum
            sbSendTelegram[0] &= ~SB_TEL_REPEAT_FLAG;
            sbSendTelegram[sbSendTelegramLen - 1] ^= SB_TEL_REPEAT_FLAG;
        }
        ++sbSendTelegramTries;

        LPC_TMR16B1_MR_OUT = sbTimeBitWait;  // Set the output to 1 after the wait time
        LPC_TMR16B1->MR3 = sbTimeBit;   // Interrupt after bit time
        val = LPC_IOCON_BUS_OUT | BUS_OUT_IOCON_PWM;

        if (sbState != SB_SEND_INIT) // do nothing if the bus is busy
            return;

        LPC_TMR16B1->TC = sbTimeBitWait;// Change the timer to have a 1 almost immediately
        LPC_IOCON_BUS_OUT = val;        // Configure bus-out pin as PWM output
        LPC_TMR16B1->MCR = 0x600;		// Interrupt and reset timer on match of MR3
        LPC_TMR16B1->CCR = 2;           // Capture CR0 on falling edge, without interrupt
        GPIOSetValue(3, 2, 1);          // yellow: start of byte
        sbState = SB_SEND_BIT_0;
        sbNextByte = 0;
        break;

    case SB_SEND_START:
        // No break for now

    case SB_SEND_BIT_0:
        sbState = SB_SEND_BYTE;

        if (sbSendAck)
            sbCurrentByte = sbSendAck;
        else sbCurrentByte = sbSendTelegram[sbNextByte++];

        // Calculate the parity bit
        for (bitMask = 1; bitMask < 0x100; bitMask <<= 1)
        {
            if (sbCurrentByte & bitMask) sbCurrentByte ^= 0x100;
        }
        bitMask = 1;
        // no break here

    case SB_SEND_BYTE:
        GPIOSetValue(3, 2, 1);      // yellow: send next bits
        timer = sbTimeBit;
        while ((sbCurrentByte & bitMask) && bitMask <= 0x100)
        {
            bitMask <<= 1;
            timer += sbTimeBit;
        }
        bitMask <<= 1;

        if (bitMask > 0x200)
        {
            timer += sbTimeBit * 3; // Stop bit + inter-byte timeout

            if (sbNextByte < sbSendTelegramLen && !sbSendAck)
            {
                sbState = SB_SEND_BIT_0;
            }
            else
            {
                sbState = SB_SEND_END;
                LPC_IOCON_BUS_OUT &= ~(LPC_IOCON_BUS_OUT | BUS_OUT_IOCON_PWM); // Disable bus-out output
            }
        }
        LPC_TMR16B1->MR3 = timer;	// Reset and interrupt at the next 0 bit
        LPC_TMR16B1_MR_OUT = timer - sbTimeBitPulse;
        break;

    case SB_SEND_END:
        sbSendAck = 0;
        // no break here

    default:
        sb_idle();
        break;
    }

    LPC_TMR16B1->IR = 0xff;
}

/**
 * Send the telegram that is stored in sbSendTelegram[].
 *
 * @param length - the length of the telegram in sbSendTelegram[], without the checksum
 */
void sb_send_tel(unsigned short length)
{
    unsigned char checksum = 0xff;
    unsigned short i;

    // Set the sender address
    sbSendTelegram[1] = sbOwnPhysicalAddr >> 8;
    sbSendTelegram[2] = sbOwnPhysicalAddr;

    // Calculate the checksum
    for (i = 0; i < length; ++i)
        checksum ^= sbSendTelegram[i];
    sbSendTelegram[length] = checksum;
    sbSendTelegramLen = length + 1;
    sbSendTelegramTries = 0;

    // Start sending if the bus is idle
    if (sbState == SB_IDLE && !(LPC_TMR16B1->IR & 0x10))
    {
        sbState = SB_SEND_INIT;
        LPC_TMR16B1->MR3 = 1;
        LPC_TMR16B1->MCR = 0x600;       // Interrupt and reset timer on match of MR3
    }
}

/**
 * Initialize the bus access.
 */
void sb_init_bus()
{
    sbRecvTelegramLen = 0;
    sbSendTelegramLen = 0;

    sbState = SB_IDLE;
    sbSendAck = 0;

    sbStatus = 0x2e;
    LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 8; // Enable the clock for the timer


    //
    // Init GPIOs for bus access
    //

    // Configure bus input
    LPC_GPIO[1]->DIR &= ~0x100;
    LPC_IOCON->PIO1_8 = 0x21;	// 0x21: Select timer1_16 CAP0, input, no pull-up/down, hysteresis
                                // 0x29: Select timer1_16 CAP0, input, pull-down, hysteresis
                                // 0x50: Select timer1_16 CAP0, input, pull-up, hysteresis
                                // 0x0a: Select timer1_16 CAP0, input, pull-down, no hysteresis

    // Configure bus output
    LPC_TMR16B1_MR_OUT = 0;		// Set bus-out match to 0 to have always 1
    LPC_TMR16B1->EMR = 0;       // Clear timer match for bus-out
#ifdef SB_BUS_OUT_P1_9
//    LPC_TMR16B1->EMR = 0x21;	// Set output to 1 on match for bus-out
    LPC_IOCON_BUS_OUT = 0;      // Set pin to normal output
    LPC_GPIO[1]->DIR |= 0x200;  // Set bus-out pin to output
    LPC_TMR16B1->PWMC = 1;      // Enable PWM for bus-out
#else // use P1_10 for bus-out
//    LPC_TMR16B1->EMR = 0x82;	// Set output to 1 on match for bus-out
    LPC_IOCON_BUS_OUT = 0x90;	// Set pin to normal output
    LPC_GPIO[1]->DIR |= 0x400;	// Set bus-out pin to output
    LPC_TMR16B1->PWMC = 2;		// Enable PWM for bus-out
#endif
    GPIOSetValue(BUS_OUT_PORT_PIN, 0);

    LPC_TMR16B1->CCR = 6;		// Capture CR0 on falling edge, with interrupt
    LPC_TMR16B1->MCR = 0;		// Do not handle timer matches
    LPC_TMR16B1->TCR = 1;		// Enable the timer
    NVIC_EnableIRQ(TIMER_16_1_IRQn); // Enable the timer interrupt


    //
    // Calculate timer ticks
    //
    unsigned short usecTicks = SystemCoreClock / 1000000;

    // TODO Use the highest possible prescaler
//    LPC_TMR16B1->PR = 1;
//    usecTicks >>= 1;
    int i;
    for (i = 0; !(usecTicks & 1); ++i)
        usecTicks >>= 1;
    LPC_TMR16B1->PR = (1 << i) - 1;

    // Calculate timer waits
    sbTimeBit = 104 * usecTicks;
    sbTimeBitWait = 69 * usecTicks;
    sbTimeBitPulse = 35 * usecTicks;
    sbTimeByte = (10 * 104 + 50) * usecTicks;


    //
    // Init GPIOs for debugging
    //
    GPIOSetDir(3, 0, 1);  // 0: input, 1: output
    GPIOSetDir(3, 1, 1);  // 0: input, 1: output
    GPIOSetDir(3, 2, 1);  // 0: input, 1: output
    GPIOSetDir(3, 3, 1);  // 0: input, 1: output
    GPIOSetDir(0, 6, 1);

    GPIOSetValue(3, 0, 0);
    GPIOSetValue(3, 1, 0);
    GPIOSetValue(3, 2, 0);
    GPIOSetValue(0, 6, 0);
}
