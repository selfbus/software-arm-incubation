// sb_bus.c

#include "sb_bus.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif
#include "../driver/gpio.h"


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

// Default time between two bits (104 usec)
static unsigned short sbTimeBit;

// Time between two bits (69 usec)
static unsigned short sbTimeBitWait;

// Duration of a bit pulse (35 usec)
static unsigned short sbTimeBitPulse;

// Maximum time from start bit to stop bit, including a safety extra
static unsigned short sbTimeRecvByte;

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
 * Timer16B0 interrupt handler
 */
void TIMER16_0_IRQHandler()
{
	static unsigned short bitMask;
	static unsigned short tick = 0;
	static unsigned short bitTime = 0;
	static unsigned char parity, checksum;
	unsigned short timer = LPC_TMR16B0->CR0;

	// Debug output
	GPIOSetValue(0, 6, ++tick & 1); 	// brown: interrupt tick
	GPIOSetValue(3, 0, (LPC_TMR16B0->IR & 0x10) != 0); // red: no falling edge since last interrupt
	GPIOSetValue(3, 1, 0);				// orange: parity bit ok
	GPIOSetValue(3, 2, 0);				// yellow: end of byte


	switch (sbState)
	{
	case SB_IDLE:
		sbNextByte = 0;
		checksum = 0xff;
		LPC_GPIO[2]->DIR &= ~(1 << 2); // Set bus-out pin to input
		// no break here

	case SB_RECV_START:
		if (LPC_TMR16B0->IR & 0x01)	// Timeout while waiting for next start byte
		{
			if (!checksum)
				sbRecvTelegramLen = sbNextByte;

			LPC_TMR16B0->MCR = 0;	// 0: Do not match
			sbState = SB_IDLE;
			break;
		}

		LPC_TMR16B0->MR0 = sbTimeRecvByte;
		LPC_TMR16B0->TCR = 2;  // Reset the timer
		LPC_TMR16B0->TCR = 1;  // Enable the timer
		LPC_TMR16B0->MCR = 3;  // 3: Interrupt and reset timer on match of MR0
		sbState = SB_RECV_BYTE;
		sbCurrentByte = 0;
		bitTime = 0;
		bitMask = 1;
		parity = 1;
		break;

	case SB_RECV_BYTE: 			// DEBUG info: first expected byte is 0xbc (10111100b, parity 1)
		if (LPC_TMR16B0->IR & 0x01)
			timer = sbTimeRecvByte;

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

		if (LPC_TMR16B0->IR & 0x01)  // Timer timeout: end of byte
		{
			GPIOSetValue(3, 2, 1);		// yellow: end of byte
			GPIOSetValue(3, 1, parity);	// orange: parity bit ok

			if (sbNextByte < SB_TELEGRAM_SIZE)
			{
				sbRecvTelegram[sbNextByte++] = sbCurrentByte;
				checksum ^= sbCurrentByte;
			}

			sbState = SB_RECV_START;				// wait for the next byte's start bit
			LPC_TMR16B0->MR0 = sbTimeRecvByte << 2; // timeout for waiting
		}
		break;

	case SB_SEND_INIT:
		if (sbSendTelegramTries == 1)
		{
			// If it is the first repeat, then mark the telegram as being repeated and correct the checksum
			sbSendTelegram[0] &= ~SB_TEL_REPEAT_FLAG;
			sbSendTelegram[sbSendTelegramLen - 1] ^= SB_TEL_REPEAT_FLAG;
		}

		if (sbState != SB_SEND_INIT || (LPC_TMR16B0->IR & 0x10)) // do nothing if the bus is busy
			return;

		GPIOSetValue(2, 2, 1);
		LPC_GPIO[2]->DIR |= 1 << 2; // Set bus-out pin to output
		// no break here

	case SB_SEND_START:
		GPIOSetValue(2, 2, 0);  // Start bit
		break;

	default:
		break;
	}

	LPC_TMR16B0->IR = 0x11;
}


/**
 * Process the received telegram in sbTelegram[].
 * Afterwards, sbTelegramLen is zero again.
 */
void sb_process_tel()
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

	sbSendTelegramLen = length + 1;
	sbSendTelegramTries = 0;

	// Start sending if the bus is idle
	if (sbState == SB_IDLE && !(LPC_TMR16B0->IR & 0x10))
	{
		sbState = SB_SEND_INIT;
		TIMER16_0_IRQHandler();
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

	//
	// Init bus timer
	//

    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);

	// Calculate ticks per microsecond
	unsigned short usecTicks = SystemCoreClock / 1000000;

	// TODO Use the highest possible prescaler
	LPC_TMR16B0->PR = 0;
//	int i;
//	for (i = 0; !(usecTicks & 1); ++i)
//		usecTicks >>= 1;
//	LPC_TMR16B0->PR = (1 << i) - 1;
//	usecTicks >>= 1;
//	LPC_TMR16B0->PR = 1;


	// Calculate timer waits
	sbTimeBit = 104 * usecTicks;
	sbTimeBitWait = 69 * usecTicks;
	sbTimeBitPulse = 35 * usecTicks;
	sbTimeRecvByte = (10 * 104 + 50) * usecTicks;

	LPC_TMR16B0->CCR = 6;		// 6: Capture CR0 on falling edge, with interrupt
	LPC_TMR16B0->MCR = 0;		// 0: Do not match the timer, 2: Reset timer on match of MR0
	LPC_TMR16B0->MR0 = sbTimeRecvByte;

    NVIC_EnableIRQ(TIMER_16_0_IRQn); // Enable the timer interrupt
    LPC_TMR16B0->TCR = 1; // Enable the timer

	//
	// Init GPIOs for bus access
	//

    // Bus input
    LPC_GPIO[0]->DIR &= ~(1 << 0);
	LPC_IOCON->PIO0_2 = 0x42;	// 0x42: Select timer0_16 CAP0, input, no pull-up/down, hysteresis
	                         	// 0x4a: Select timer0_16 CAP0, input, pull-down, hysteresis
								// 0x50: Select timer0_16 CAP0, input, pull-up, hysteresis
    							// 0x0a: Select timer0_16 CAP0, input, pull-down, no hysteresis

	// Bus output
	LPC_GPIO[2]->DIR &= ~(1 << 2);// Set bus-out pin to input
	LPC_IOCON->PIO2_2 = 0x000;	// 0x000: Standard pin, normal output
								// 0x400: Standard pin, open-drain output

	//
	// Init GPIOs for debugging
	//
	GPIOSetDir(3, 0, 1);  // 0: input, 1: output
	GPIOSetDir(3, 1, 1);  // 0: input, 1: output
	GPIOSetDir(3, 2, 1);  // 0: input, 1: output
	GPIOSetDir(0, 6, 1);

    GPIOSetValue(3, 0, 0);
    GPIOSetValue(3, 1, 0);
    GPIOSetValue(3, 2, 0);
	GPIOSetValue(0, 6, 0);
}
