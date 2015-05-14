/*
 *  timer.cpp - Timer manipulation and time functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/timer.h>

#include <sblib/internal/variables.h>


// The number of milliseconds since processor start/reset
volatile unsigned int systemTime;

// The timers
static LPC_TMR_TypeDef* const timers[4] = { LPC_TMR16B0, LPC_TMR16B1, LPC_TMR32B0, LPC_TMR32B1 };


void delay(unsigned int msec)
{
    unsigned int lastSystemTime = systemTime;

    while (msec)
    {
        if (lastSystemTime == systemTime)
        {
            __WFI();
        }
        else
        {
            lastSystemTime = systemTime;
            --msec;
        }
    }
}

#ifndef IAP_EMULATION
void delayMicroseconds(unsigned int usec)
{
    unsigned int val, lastVal = SysTick->VAL;
    int ticksToWait = (usec - 2) * (SystemCoreClock / 1000000);
    int elapsed;

    // SysTick is counting down and is reset to SysTick->LOAD when
    // it reaches zero.

    while (ticksToWait > 0)
    {
        val = SysTick->VAL;

        elapsed = lastVal - val;
        if (elapsed < 0)
            elapsed += SysTick->LOAD;

        ticksToWait -= elapsed;
        lastVal = val;
    }
}
#endif

//----- Class Timer -----------------------------------------------------------

Timer timer16_0(TIMER16_0);
Timer timer16_1(TIMER16_1);
Timer timer32_0(TIMER32_0);
Timer timer32_1(TIMER32_1);


Timer::Timer(byte aTimerNum)
{
    timerNum = aTimerNum;
    timer = timers[aTimerNum];
}

void Timer::begin()
{
    LPC_SYSCON->SYSAHBCLKCTRL |= 1 << (7 + timerNum);

    timer->EMR = 0;
    timer->MCR = 0;
    timer->CCR = 0;
}


void Timer::matchMode(int channel, int mode)
{
    int offset;

    // Configure the match control channel

    offset = channel * 3;
    timer->MCR &= ~(7 << offset);
    timer->MCR |= (mode & 7) << offset;

    // Configure the external match channel
    matchModePinConfig(channel, mode);
}

int Timer::matchMode(int channel) const
{
    int mode, offset;

    // Query the match control channel

    offset = channel * 3;
    mode = (timer->MCR >> offset) & 7;

    // Query the external match channel

    offset = channel << 1;
    mode |= (timer->EMR >> offset) & 0x30;

    return mode;
}

void Timer::captureMode(int channel, int mode)
{
    short offset = channel * 3;

    short val = (mode >> 6) & 3;
    if (mode & INTERRUPT)
        val |= 4;

    timer->CCR &= ~(7 << offset);
    timer->CCR |= val << offset;
}

int Timer::captureMode(int channel) const
{
    int mode = ((timer->CCR >> (channel * 3)) & 7) << 6;

    if (mode & 0x100)
    {
        mode &= 0xc0;
        mode |= INTERRUPT;
    }

    return mode;
}

void Timer::counterMode(int mode, int clearMode)
{
    int config = 0;

    if (mode & RISING_EDGE)
        config |= 0x1;
    if (mode & FALLING_EDGE)
        config |= 0x2;
    if (mode & CAP1)
        config |= 0x4;

    if (clearMode)
    {
        config |= 0x10;

        if (clearMode & (CAP1 | FALLING_EDGE))
            config |= 0x60;
        else if (clearMode & (CAP1 | RISING_EDGE))
            config |= 0x40;
        else if (clearMode & (CAP0 | FALLING_EDGE))
            config |= 0x20;
        // CAP0 | RISING_EDGE is 0x00
    }

    timer->CTCR = config;
}

//
// The original timer handler is used for performance reasons.
// Use attachInterrupt() to override this handler.
//
extern "C" void SysTick_Handler()
{
    ++systemTime;
}
