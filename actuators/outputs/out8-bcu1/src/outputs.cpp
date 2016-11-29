/*
 *  outputs.cpp - Handle the update of the port pins in accordance to the
 *                need of the application
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "outputs.h"
#include <sblib/io_pin_names.h>

#ifndef BI_STABLE
Outputs relays;
#endif

const unsigned int zeroDetectSetDelay = 0x0200; //0x0200 Omron G5Q-1A EU 10A set
const unsigned int zeroDetectClrDelay = 0x0240; //0x0220 Omron G5Q-1A EU 10A clear

#define ZD_RESET ((zeroDetectSetDelay > zeroDetectClrDelay ? zeroDetectSetDelay : zeroDetectClrDelay) + 1)


void Outputs::begin (unsigned int initial, unsigned int inverted)
{
#ifndef BI_STABLE
    pinMode(PIN_PWM, OUTPUT_MATCH);  // configure digital pin PIO3_2(PWM) to match MAT2 of timer16 #0
    //pinMode(PIO1_4, OUTPUT);
    timer16_0.begin();

    timer16_0.prescaler((SystemCoreClock / 100000) - 1);
    timer16_0.matchMode(MAT2, SET);  // set the output of PIO3_2 to 1 when the timer matches MAT1
    timer16_0.match(MAT2, PWM_DUTY);        // match MAT1 when the timer reaches this value
    timer16_0.pwmEnable(MAT2);       // enable PWM for match channel MAT1

    // Reset the timer when the timer matches MAT3
    timer16_0.matchMode(MAT3, RESET);
    timer16_0.match(MAT3, PWM_PERIOD);     // match MAT3 to create 14lHz
    timer16_0.start();
    _pwm_timeout.start(PWM_TIMEOUT); // start the timer to switch back to a PWM operation
#endif
    //digitalWrite(PIO1_4, 1);
    _relayState     = initial;
    _prevRelayState = ~initial;
    _inverted       = inverted;
#ifdef ZERO_DETECT
    _state           = 0; // wait for next required switch
    _port_0_set = _port_0_clr = _port_2_set = _port_2_clr = 0;
    timer32_0.begin();
    timer32_0.prescaler((SystemCoreClock / 100000) - 1);
    timer32_0.matchMode(MAT3, RESET | STOP);
    timer32_0.matchMode(MAT0, INTERRUPT);
    timer32_0.matchMode(MAT1, INTERRUPT);
    timer32_0.match(MAT0, zeroDetectSetDelay);
    timer32_0.match(MAT1, zeroDetectClrDelay);
    timer32_0.match(MAT3, ZD_RESET);
    timer32_0.interrupts();
#endif
}

void Outputs::updateOutputs(void)
{
    unsigned int mask = 0x01;
    unsigned int i;
    unsigned int state     = _relayState ^ _inverted;
    unsigned int prevState = _prevRelayState ^  _inverted;
    unsigned int toggle    = pendingChanges();

#ifndef ZERO_DETECT
    if ((state ^ prevState) & state)
    {   // at least one outputs needs to be switched ON -> disable
        // the PWM
        timer16_0.match(MAT2, PWM_PERIOD);// disable the PWM
        _pwm_timeout.start(PWM_TIMEOUT);
    }
#endif
    for(i = 0; i < NO_OF_CHANNELS; i++, mask <<= 1)
    {
    	unsigned int value = state & mask;
    	if (mask & toggle)
    	{
    		unsigned int pinMask = digitalPinToBitMask(outputPins[i]);
			if (digitalPinToPort(outputPins[i]) != 0)
			{
				if (value) _port_2_set |= pinMask;
				else       _port_2_clr |= pinMask;
			}
			else
			{
				if (value) _port_0_set |= pinMask;
				else       _port_0_clr |= pinMask;
			}
#ifdef HAND_ACTUATION
			handAct.setLedState(i, value);
#endif
    	}
    }
    _prevRelayState = _relayState;
#ifdef ZERO_DETECT
    _state = 1;
#else
    setOutputs();
    clrOutputs();
#endif
}

#ifdef ZERO_DETECT
extern "C" void PIOINT0_IRQHandler (void)
{
    LPC_GPIO_TypeDef* port = gpioPorts[0];
    port->IC  =  1<<5; // clear the interrupt
    relays.zeroDetectHandler();
}

extern "C" void TIMER32_0_IRQHandler(void)
{
    if (timer32_0.flags () & 0x01)
    {   // handle SET ports
    	relays.setOutputs();
    }
    if (timer32_0.flags () & 0x02)
    {   // handle CLR ports
    	relays.clrOutputs();
    }
    timer32_0.resetFlags();
    digitalWrite(PIN_INFO, ! digitalRead(PIN_INFO));
}
#endif
