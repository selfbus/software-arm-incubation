// sb_bus.c

#include "sb_bus.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif
#include "../driver/gpio.h"

#define PROTOCOL_DEBUG

#define SB_TIMER_NO  0
#define SB_PORT      0
#define SB_PIN_IN    2
#define SB_PIN_OUT   9

#if   (SB_PORT == 0) && (SB_PIN_IN == 2)
// 0x22: Select timer0_16 CAP0, input, no pull-up/down, hysteresis
#define INPUT_IOCON() LPC_IOCON->PIO0_2 = 0x22
#elif (SB_PORT == 1) && (SB_PIN_IN == 8)
// 0x21: Select timer1_16 CAP0, input, no pull-up/down, hysteresis
// 0x29: Select timer1_16 CAP0, input, pull-down, hysteresis
// 0x50: Select timer1_16 CAP0, input, pull-up, hysteresis
// 0x0a: Select timer1_16 CAP0, input, pull-down, no hysteresis
#define INPUT_IOCON() LPC_IOCON->PIO1_8 = 0x21;
#else
#error "Unknow input pin config"
#endif

// 0x0: Standard pin, normal output
// 0x400: Standard pin, open-drain output
#if   (SB_PORT == 0) && (SB_PIN_OUT == 9)
#define OUTPUT_IOCON() LPC_IOCON->PIO0_8 = 0x0
#elif (SB_PORT == 1) && (SB_PIN_OUT == 9)
#define OUTPUT_IOCON() LPC_IOCON->PIO1_9 = 0x0
#else
#error "Unknow input pin config"
#endif

#if SB_TIMER_NO == 0
#define TIMER_IRQ_NO TIMER_16_0_IRQn
#define TIMER_IRQ    TIMER16_0_IRQHandler
#define TIMER_REG    LPC_TMR16B0
#else
#define TIMER_IRQ_NO TIMER_16_1_IRQn
#define TIMER_IRQ    TIMER16_1_IRQHandler
#define TIMER_REG    LPC_TMR16B1
#endif

#define OUT_MASK (1 << SB_PIN_OUT)
#define IN_MASK  (1 << SB_PIN_IN)

// Calculate timer ticks per microsecond
#define SB_USEC_TICK        (SystemCoreClock / 1000000)
// Calculate timer waits
#define SB_TIME_BIT         (104 * SB_USEC_TICK)
#define SB_TIME_BIT_WAIT    ( 69 * SB_USEC_TICK)
#define SB_TIME_BIT_PULSE   ( 35 * SB_USEC_TICK)
#define SB_TIME_RECV_BYTE   ( 10 * SB_TIME_BIT + 50 * SB_USEC_TICK)

#define TIMER_MATCH_INTERRUPT    (0x08)
#define TIMER_CAPTURE_INTERRUPT  (0x10)

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


//----- private variables -----
// The current byte that is received/sent
static unsigned short sbCurrentByte;

// The number of the next byte in the telegram
static short sbNextByte;

// Our own physical address: 1.1.128
static unsigned short sbOwnPhysicalAddr = 0x1180;

// The number of repeats when sending a telegram
static short sbSendTelegramTries;

//----- Telegram bits and constants -----

// Telegram repeat flag in byte #0 of the telegram: 1=not repeated, 0=repeated
#define SB_TEL_REPEAT_FLAG 0x20

/**
 * Timer16 #1 interrupt handler
 */
void TIMER_IRQ(void)
{
    static unsigned short bitMask;
    static unsigned short bitTime  = 0;
    static unsigned char parity, checksum;
    unsigned short timer = TIMER_REG->CR0;

#ifdef PROTOCOL_DEBUG
    static unsigned short tick     = 0;
    GPIOSetValue(0, 6, ++tick & 1);     // brown: interrupt tick
    GPIOSetValue(3, 0, (TIMER_REG->IR & TIMER_CAPTURE_INTERRUPT) != 0); // red: no falling edge since last interrupt
    GPIOSetValue(3, 1, 0);              // orange: parity bit ok
    GPIOSetValue(3, 2, 0);              // yellow: end of byte
#endif
    switch (sbState)
    {
    case SB_IDLE:
        sbNextByte = 0;
        checksum   = 0xff;
        LPC_GPIO [SB_PORT]->DIR &= ~OUT_MASK; // Set bus-out pin to input
        // no break here

    case SB_RECV_START:
        if (TIMER_REG->IR & TIMER_MATCH_INTERRUPT) // Timeout while waiting for next start byte
        {
            if (!checksum) // checksum is valid -> mark the a valid packed has been received
                sbRecvTelegramLen = sbNextByte;

            TIMER_REG->MCR = 0;   // 0: Do not match
            sbState        = SB_IDLE;
            break;
        }

        TIMER_REG->MR3 = SB_TIME_RECV_BYTE;
        TIMER_REG->TCR = 2;  // Reset the timer
        TIMER_REG->TCR = 1;  // Enable the timer
        TIMER_REG->MCR = 3 << 9;  // 3: Interrupt and reset timer on match of MR0
        sbState        = SB_RECV_BYTE;
        sbCurrentByte  = 0;
        bitTime        = 0;
        bitMask        = 1;
        parity         = 1;
        break;

    case SB_RECV_BYTE:          // DEBUG info: first expected byte is 0xbc (10111100b, parity 1)
        if (TIMER_REG->IR & TIMER_MATCH_INTERRUPT)
            timer = SB_TIME_RECV_BYTE;
        if (timer >= (bitTime + SB_TIME_BIT_WAIT))
        {
            bitTime += SB_TIME_BIT;
            while (   timer >= (bitTime + SB_TIME_BIT_WAIT)
            	  && (bitMask <= 0x100)
            	  )
            {
                sbCurrentByte |= bitMask;
                parity         = !parity;
                bitTime       += SB_TIME_BIT;
                bitMask      <<= 1;
            }
            bitMask <<= 1;
        }

        if (TIMER_REG->IR & TIMER_MATCH_INTERRUPT)  // Timer timeout: end of byte
        {
#ifdef PROTOCOL_DEBUG
            GPIOSetValue(3, 2, 1);      // yellow: end of byte
            GPIOSetValue(3, 1, parity); // orange: parity bit ok
#endif
            if (sbNextByte < SB_TELEGRAM_SIZE)
            {
                sbRecvTelegram[sbNextByte++] = sbCurrentByte;
                checksum                    ^= sbCurrentByte;
            }
            sbState                       = SB_RECV_START;          // wait for the next byte's start bit
            TIMER_REG->MR3 = SB_TIME_RECV_BYTE << 2; // timeout for waiting
        }
        break;

    case SB_SEND_INIT:
        if (sbSendTelegramTries == 1)
        {
            // If it is the first repeat, then mark the telegram as being repeated and correct the checksum
            sbSendTelegram[0]                     &= ~SB_TEL_REPEAT_FLAG;
            sbSendTelegram[sbSendTelegramLen - 1] ^=  SB_TEL_REPEAT_FLAG;
        }
        // do nothing if the bus is busy
        if (sbState != SB_SEND_INIT || (TIMER_REG->IR & TIMER_CAPTURE_INTERRUPT))
            return;

#ifdef PROTOCOL_DEBUG
        GPIOSetValue(2, 2, 1);
#endif
        LPC_GPIO[2]->DIR |= 1 << 2; // Set bus-out pin to output
        // no break here

    case SB_SEND_START:
        GPIOSetValue(2, 2, 0);  // Start bit
        break;

    default:
        break;
    }
    TIMER_REG->IR = TIMER_MATCH_INTERRUPT | TIMER_CAPTURE_INTERRUPT;
}

/**
 * Process the received telegram in sbTelegram[].
 * Afterwards, sbTelegramLen is zero again.
 */
void sb_process_tel(void)
{
    // TODO

    // At end: mark the telegram buffer as empty
    sbRecvTelegramLen = 0;
}


/**
 * Prepare sending the telegram of sbSendTelegram[].
 */
inline static void sb_send_tel_prepare()
{
    ++sbSendTelegramTries;
}


/**
 * Send the telegram that is stored in sbSendTelegram[].
 *
 * @param length - the length of the telegram in sbSendTelegram[], without the checksum
 */
void sb_send_tel(unsigned short length)
{
    unsigned char checksum = 0;
    unsigned short i;

    // Set the sender address
    sbSendTelegram[1] = sbOwnPhysicalAddr >> 8;
    sbSendTelegram[2] = sbOwnPhysicalAddr & 255;

    // Calculate the checksum
    for (i = 0; i < length; ++i)
        checksum ^= sbSendTelegram[i];
    sbSendTelegram[length] = checksum;
    sbSendTelegramLen      = length + 1;
    sbSendTelegramTries    = 0;

    // Start sending if the bus is idle
    if (sbState == SB_IDLE && !(TIMER_REG->IR & TIMER_CAPTURE_INTERRUPT))
    {
        sbState = SB_SEND_INIT;
        TIMER_IRQ ();
    }
}

/**
 * Initialize the bus access.
 */
void sb_init_bus(void)
{
    sbRecvTelegramLen = 0;
    sbSendTelegramLen = 0;
    sbState           = SB_IDLE;
    //
    // Init GPIOs for bus access
    //

    LPC_GPIO[SB_PORT]->DIR &= ~IN_MASK; // Set bus-in  pin to input
    INPUT_IOCON  ();
    LPC_GPIO[SB_PORT]->DIR &= ~OUT_MASK;// Set bus-out pin to input
    OUTPUT_IOCON ();
    //
    // Init bus timer
    //
    LPC_SYSCON->SYSAHBCLKCTRL |= 1<< (7 + SB_TIMER_NO);  // Enable the clock for the timer

    // TODO Use the highest possible prescaler
    TIMER_REG->PR = 0;
//  int i;
//  for (i = 0; !(USECTICKS & 1); ++i)
//      USECTICKS >>= 1;
//  TIMER_REG->PR = (1 << i) - 1;
//  USEC_TICK >>= 1;

    TIMER_REG->CCR = 6;       // 6: Capture CR0 on falling edge, with interrupt
    TIMER_REG->MCR = 3 << 9;  // 0: Do not match the timer, 2: Reset timer on match of MR0, 3: Interrupt and reset
    TIMER_REG->MR3 = SB_TIME_RECV_BYTE;

    TIMER_REG->TCR = 1;               // Enable the timer
    NVIC_EnableIRQ(TIMER_IRQ_NO); // Enable the timer interrupt
#ifdef PROTOCOL_DEBUG
    //
    // Init GPIOs for debugging
    //
    GPIOSetDir(3, 0, 1);
    GPIOSetDir(3, 1, 1);
    GPIOSetDir(3, 2, 1);
    GPIOSetDir(0, 6, 1);

    GPIOSetValue(3, 0, 0);
    GPIOSetValue(3, 1, 0);
    GPIOSetValue(3, 2, 0);
    GPIOSetValue(0, 6, 0);
#endif
}
