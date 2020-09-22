/*
 * debug.h
 *
 *  Created on: 11.09.2020
 *      Author: denis
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG

#include <sblib/timeout.h>

Timeout TX_FlashTimeOut;
Timeout RX_FlashTimeOut;


/*
 * Handler for the timer_0 interrupt.
 */
/*
extern "C" void TIMER32_0_IRQHandler()
{
    // Toggle the Info LED
    digitalWrite(RX_LED, LED_OFF);

    // Clear the timer interrupt flags. Otherwise the interrupt handler is called
    // again immediately after returning.
    timer32_0.resetFlags();
    timer32_0.end();
}
*/
/*
 * Handler for the timer_1 interrupt.
 */
/*
extern "C" void TIMER32_1_IRQHandler()
{
    // Toggle the Info LED
    digitalWrite(TX_LED, !digitalRead(TX_LED));

    // Clear the timer interrupt flags. Otherwise the interrupt handler is called
    // again immediately after returning.
    timer32_1.resetFlags();
    timer32_1.end();
}
*/

void FlashRX_LED(int FlashTime_ms = RX_FLASH_MS)
{
    pinMode(RX_LED, OUTPUT);
    digitalWrite(RX_LED, LED_ON);
    RX_FlashTimeOut.stop();
    RX_FlashTimeOut.start(FlashTime_ms);

    /*
    // Enable the timer interrupt
    enableInterrupt(TIMER_32_0_IRQn);

    // Begin using the timer
    timer32_0.begin();

    // Let the timer count milliseconds
    timer32_0.prescaler((SystemCoreClock / 1000) - 1);

    // On match of MAT1, generate an interrupt and reset the timer
    timer32_0.matchMode(MAT1, RESET | INTERRUPT);

    // Match MAT1 when the timer reaches this value (in milliseconds)
    timer32_0.match(MAT1, RX_FLASH_MS);

    timer32_0.start();
    */
}

void FlashTX_LED(int FlashTime_ms = TX_FLASH_MS)
{
    pinMode(TX_LED, OUTPUT);
    digitalWrite(TX_LED, LED_ON);
    TX_FlashTimeOut.stop();
    TX_FlashTimeOut.start(FlashTime_ms);
    /*
    // Enable the timer interrupt
    enableInterrupt(TIMER_32_1_IRQn);

    // Begin using the timer
    timer32_1.begin();

    // Let the timer count milliseconds
    timer32_1.prescaler((SystemCoreClock / 1000) - 1);

    // On match of MAT1, generate an interrupt and reset the timer
    timer32_1.matchMode(MAT1, RESET | INTERRUPT);

    // Match MAT1 when the timer reaches this value (in milliseconds)
    timer32_1.match(MAT1, TX_FLASH_MS);

    timer32_1.start();
    */
}

void debugCheckPeriodic()
{
    if (TX_FlashTimeOut.expired())
    {
        digitalWrite(TX_LED, LED_OFF);
    }

    if (RX_FlashTimeOut.expired())
    {
        digitalWrite(RX_LED, LED_OFF);
    }

}

#else /* DEBUG */
void FlashRX_LED(){};
void FlashRX_LED(int FlashTime_ms){};
void FlashTX_LED(){};
void FlashTX_LED(int FlashTime_ms){};
void debugCheckPeriodic(){};
#endif



#endif /* DEBUG_H_ */
