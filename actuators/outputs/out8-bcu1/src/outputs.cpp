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

#include <sblib/io_pin_names.h>

#include "outputs.h"
#include "app_out8.h" // FIXME get rid of outputPins[]

#ifndef BI_STABLE
    Outputs relays;
#endif

const unsigned int zeroDetectSetDelay = 0x0200; //0x0200 Omron G5Q-1A EU 10A set
const unsigned int zeroDetectClrDelay = 0x0240; //0x0220 Omron G5Q-1A EU 10A clear

#define ZD_RESET ((zeroDetectSetDelay > zeroDetectClrDelay ? zeroDetectSetDelay : zeroDetectClrDelay) + 1)


void Outputs::begin (unsigned int initial, unsigned int inverted, unsigned int channelcount)
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
    _channelcount   = channelcount;
    _relayState     = initial;
    _prevRelayState = (~_relayState) & ((1 << _channelcount) -1);
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

/*
 * returns true in case a switching action was started which drained the bus
 */
unsigned int Outputs::updateOutput(unsigned int channel)
{
    unsigned int mask      = 1 << channel;
    unsigned int state     = (mask & (_relayState ^ _inverted)) >> channel;
    unsigned int prevState = (mask & (_prevRelayState ^  _inverted)) >> channel;
    unsigned int value     = (state ^ prevState) & state;
    unsigned int result    = false;

    if (!(state ^ prevState))
        return false; // nothing to do

#ifndef ZERO_DETECT
    if (value)
    {
        // output needs to be switched ON -> disable the PWM
        timer16_0.match(MAT2, PWM_PERIOD);
        _pwm_timeout.start(PWM_TIMEOUT);
        result = true;
    }
#endif

    unsigned int pinMask = digitalPinToBitMask(outputPins[channel]);
    if (digitalPinToPort(outputPins[channel]) != 0)
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
    handAct.setLedState(channel, value);
#endif

    _prevRelayState ^= mask; // toggle the bit of the channel we changed

#ifdef ZERO_DETECT
    _state = 1;
#else
    setOutputs();
    clrOutputs();
#endif

    return result;
}

void Outputs::updateOutputs(unsigned int delayms)
{
    for(unsigned int i = 0; i < _channelcount; i++)
    {
        if (updateOutput(i))
            delay(delayms);
    }
}

unsigned int Outputs::channelCount()
{
    return _channelcount;
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
