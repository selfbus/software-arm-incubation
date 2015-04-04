/*
 *  timer.h - Timer manipulation and time functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_timer_h
#define sblib_timer_h

#include <sblib/platform.h>
#include <sblib/types.h>

class Timer;


/**
 * Delay the program execution by sleeping a number of milliseconds.
 *
 * @param msec - the number of milliseconds to sleep.
 */
void delay(unsigned int msec);

/**
 * Delay the program execution a number of microseconds. You should use delay() when
 * possible, as this function does a busy wait which consumes more power than delay().
 * The minimum delay is roughly 3 usec. Everything below will result in a 3 usec delay.
 *
 * @param usec - the number of microseconds to wait.
 */
void delayMicroseconds(unsigned int usec);

/**
 * Get the number of milliseconds that elapsed since the last reset or processor start.
 * Please note that the system time overflows and restarts at zero after 49,7 days.
 *
 * @return The number of milliseconds.
 */
unsigned int millis();

/**
 * Get the number of milliseconds that elapsed since the reference time.
 *
 * @param ref - the reference time to compare with
 * @return The numer of milliseconds since time.
 */
unsigned int elapsed(unsigned int ref);

/**
 * The number of CPU clock cycles per microsecond.
 */
#define clockCyclesPerMicrosecond() (SystemCoreClock / 1000000)

/**
 * Convert the clock cycles cyc to microseconds.
 *
 * @param cyc - the clock cycles to convert.
 * @return The microseconds it takes to get cyc clock cycles.
 */
#define clockCyclesToMicroseconds(cyc) ((cyc) / clockCyclesPerMicrosecond())

/**
 * Convert the microseconds msec to clock cycles.
 *
 * @param msec - the microseconds to convert.
 * @return The clock cycles it takes to get msec microseconds.
 */
#define microsecondsToClockCycles(msec) ((msec) * clockCyclesPerMicrosecond())


/**
 * The 16bit timer #0.
 */
extern Timer timer16_0;

/**
 * The 16bit timer #1.
 */
extern Timer timer16_1;

/**
 * The 32bit timer #0.
 */
extern Timer timer32_0;

/**
 * The 32bit timer #1.
 */
extern Timer timer32_1;


/**
 * A timer.
 *
 * The timer is free running, with either 16 bit or 32 bit resolution. Up to 4 match channels
 * can control the timer or change digital pins of the processor. The timer is incremented with
 * every cycle of the peripheral clock. The prescaler can be used to divide the peripheral
 * clock before it is applied to the timer.
 *
 * You need to powerOn() a timer before it can be used.
 *
 * Use the predefined timers (timer16_0, timer16_1, timer32_0, timer32_1) instead of this class.
 */
class Timer
{
public:
	/**
	 * Create a timer object for the specified timer.
	 *
	 * @param timerNum - the number of the timer: TIMER16_0, TIMER32_1, ...
	 */
	Timer(byte timerNum);

	/**
	 * Begin using the timer.
	 *
	 * This powers the timer on and the system clock is connected to the timer.
	 * This method must be called before any other method of the timer can be used.
	 */
	void begin();

    /**
     * End using the timer.
     *
     * This removes the system clock from the timer. The timer becomes unusable and
     * cannot be accessed with any other timer method until begin() is called again.
     */
    void end();

	/**
	 * Set the prescaler. The prescaler can be used to divide the peripheral clock before it is
	 * applied to the timer. The factor may be 16 bit for a 16 bit timer, and 32 bit for a
	 * 32 bit timer. The prescaler causes the timer to increment on every peripheral clock when
	 * factor = 0, every 2 peripheral clocks when factor = 1, etc.
	 *
	 * @param factor - the prescaler division factor.
	 */
	void prescaler(unsigned int factor);

	/**
	 * Get the prescaler division factor. The factor is 16 bit for a 16 bit timer, and 32 bit
	 * for a 32 bit timer. The prescaler causes the timer to increment on every peripheral clock
	 * when factor = 0, every 2 peripheral clocks when factor = 1, etc.
	 */
	unsigned int prescaler() const;

    /**
     * Start the timer.
     */
    void start();

    /**
     * Stop the timer.
     */
    void stop();

    /**
     * Restart the timer. This is the same as calling reset() and start().
     * If the timer was not previously running, it is started.
     */
    void restart();

    /**
     * Rest the timer.
     */
    void reset();

    /**
     * Read the current value of the timer.
     *
     * @return The timer value.
     */
    unsigned int value() const;

    /**
     * Set the current value of the timer.
     *
     * @param val - The new timer value.
     */
    void value(unsigned int val);

    /**
     * Enable timer interrupts for this timer.
     * Use start() to start the timer.
     */
    void interrupts();

    /**
     * Disable timer interrupts for this timer.
     * Use stop() to stop the timer.
     */
    void noInterrupts();

    /**
     * Read the interrupt flags of the timer. This register contains the flags that caused an
     * interrupt for the timer.
     *
     * The bits are:
     *
     * 0 - interrupt flag for match channel MAT0
     * 1 - interrupt flag for match channel MAT1
     * 2 - interrupt flag for match channel MAT2
     * 3 - interrupt flag for match channel MAT3
     * 4 - interrupt flag for capture channel CAP0
     * 5 - interrupt flag for capture channel CAP1
     *
     * @see resetFlags()
     * @see flagSet(TimerMatch)
     * @see flagSet(TimerCapture)
     */
    int flags() const;

    /**
     * Reset the interrupt flags of the timer.
     */
    void resetFlags();

    /**
     * Test if the timer flag of a match channel is set.
     *
     * @param match - the match channel: MAT0, MAT1, MAT2, MAT3.
     * @return True if the flag is set, false if it is not set.
     */
    bool flag(TimerMatch match) const;

    /**
     * Test if the timer flag of a capture channel is set.
     *
     * @param cap - the capture channel: CAP0, CAP1.
     * @return True if the flag is set, false if it is not set.
     */
    bool flag(TimerCapture cap) const;

    /**
     * Get the timer flag mask for flags() for a match channel.
     *
     * @param match - the match channel: MAT0, MAT1, MAT2, MAT3.
     * @return The timer flag mask.
     */
    static int flagMask(TimerMatch match);

    /**
     * Get the timer flag mask for flags() for a capture channel.
     *
     * @param cap - the capture channel: CAP0, CAP1.
     * @return The timer flag mask.
     */
    static int flagMask(TimerCapture cap);

    /**
     * Configure a match channel.
     *
     * @param channel - the match channel to configure: MAT0, MAT1, MAT2, MAT3.
     * @param mode - the configuration. Multiple values are combined with "|".
     *
     * @brief
     * The match channels allow the following configuration options:
     *
     * DISABLE:      Disable matching for this match channel.
     * INTERRUPT:    Generate a timer interrupt when the contents of the match channel matches
     *               the value of the timer.
     * RESET:        Reset the timer on match.
     * STOP:         Stop the timer when the contents of the match channel matches the value
     *               of the timer.
     * CLEAR:        Clear the digital pin of the match channel to 0 on match.
     * SET:          Set the digital pin of the match channel to 1 on match.
     * TOGGLE:       Toggle the digital pin of the match channel.
     */
    void matchMode(int channel, int mode);

    /**
     * Get the configuration of a match channel.
     *
     * @param channel - the match channel to query: MAT0, MAT1, MAT2, MAT3.
     *
     * @return The configuration. See matchMode(short,short) above for the possible configuration values.
     */
    int matchMode(int channel) const;

    /**
     * Set the timer value for a match channel. Use matchMode() to configure what shall
     * happen when the timer matches. Use pinMode(short,short) to configure the digital pin as
     * match output pin.
     *
     * @param channel - the match channel: MAT0, MAT1, MAT2, MAT3.
     * @param value - the timer value when the match channel shall match. The value is 16 bit for
     *                16 bit timers and 32 bit for 32 bit timers.
     */
    void match(int channel, unsigned int value);

    /**
     * Get the timer value of a match channel.
     *
     * @param channel - the match channel to query: MAT0, MAT1, MAT2, MAT3.
     * @return The timer value when the match channel shall match. The value is 16 bit for 16 bit
     *         timers and 32 bit for 32 bit timers.
     */
    unsigned int match(int channel) const;

    /**
     * Configure a capture channel.
     *
     * @param channel - the channel to configure: CAP0, CAP1.
     * @param mode - the configuration. Multiple values are combined with "|".
     *
     * @brief
     * The capture channels allow the following configuration options:
     *
     * DISABLE:      Disable capturing,
     * INTERRUPT:    Generate a timer interrupt when the capture condition is met,
     * RISING_EDGE:  Capture on rising edge: a sequence of 0 then 1 on the capture pin will cause
     *               the capture channel to be loaded with the current timer value.
     * FALLING_EDGE: Capture on falling edge: a sequence of 1 then 0 on the capture pin will cause
     *               the capture channel to be loaded with the current timer value.
     */
    void captureMode(int channel, int mode);

    /**
     * Get the configuration of a capture channel.
     *
     * @param channel - the channel to query: CAP0, CAP1.
     *
     * @return The configuration. See mode(short,short) above for the possible configuration values.
     */
    int captureMode(int channel) const;

    /**
     * Get the timer value of a capture channel.
     *
     * @param channel - the channel to query: MAT0, MAT1, MAT2, MAT3
     *
     * @return The timer value when the capture occurred. The value is 16 bit for 16 bit timers
     *         and 32 bit for 32 bit timers.
     */
    unsigned int capture(int channel) const;

    /**
     * Enable a PWM channel. When enabled, the corresponding match channel (MAT0 for PWM0, ...)
     * will control the level of the digital pin of the match channel. Use pinMode(short,short)
     * to configure the digital pin as match output pin.
     *
     * @param channel - the PWM channel: PWM0, PWM1, PWM2, PWM3.
     *
     * When PWM output is enabled, the output of the match pin will be:
     * - as long as the timer value is below the match value, the output is low.
     * - when the timer value is equal or higher than the match value, the output is high.
     * - when the timer resets the output is low, except if the match value is 0.
     */
    void pwmEnable(int channel);

    /**
     * Disable a PWM channel.
     *
     * @param channel - the PWM channel: PWM0, PWM1, PWM2, PWM3.
     */
    void pwmDisable(int channel);

    /**
     * Configure the counter mode.
     *
     * @param mode - the counter mode configuration, see below.
     * @param clearMode - the counter clearing mode configuration, see below.
     *
     * The following flags can be used for mode and clearMode:
     * DISABLE: to disable counter mode (this is the default),
     * CAP0: use capture channel 0 for the counter input,
     * CAP1: use capture channel 1 for the counter input,
     * RISING_EDGE: the timer counter is incremented on every rising edge of the capture channel,
     * FALLING_EDGE: the timer counter is incremented on every falling edge of the capture channel.
     *
     * To activate the counter mode, it is mandatory to select RISING_EDGE, FALLING_EDGE, or both
     * for mode. Otherwise the result is undefined. The clearMode can be set to DISABLE.
     *
     * Use the clearing mode clearMode to reset the timer by a capture channel:
     * DISABLE: to disable clearing mode (this is the default),
     * CAP0: use capture channel 0 for clearing,
     * CAP1: use capture channel 1 for clearing,
     * RISING_EDGE: the rising edge of the selected capture channel clears the timer,
     * FALLING_EDGE: the falling edge of the selected capture channel clears the timer.
     * It is not possible to use both edges for clearing.
     *
     * Example:  timer16_0.counterMode(CAP0|RISING_EDGE, CAP1|FALLING_EDGE);
     */
    void counterMode(int mode, int clearMode);

    /**
     * Configure a match pin mode of the channel.
     *
     * @param channel - the match channel to configure: MAT0, MAT1, MAT2, MAT3.
     * @param mode - the configuration. Multiple values are combined with "|".
     *
     * @brief
     * The match channels allow the following configuration options:
     *
     * CLEAR:        Clear the digital pin of the match channel to 0 on match.
     * SET:          Set the digital pin of the match channel to 1 on match.
     * TOGGLE:       Toggle the digital pin of the match channel.
     */
    void matchModePinConfig(int channel, int mode);

    /**
     * Get the info if it is a 32bit timer
     *
     * @return true if the timer is a 32bit timer, otherwise 16bit timer
     */
    bool is32bitTimer(void);

    /**
     * Get the digital level of match channel (even if output pin is disabled)
     *
     * @param channel - the match channel to configure: MAT0, MAT1, MAT2, MAT3.
     *
     * @return - true = high, false = low
     */
    bool getMatchChannelLevel(int channel);


protected:
	LPC_TMR_TypeDef* timer;
    byte timerNum;
};


//
// Inline functions
//


ALWAYS_INLINE unsigned int millis()
{
    extern volatile unsigned int systemTime;
    return systemTime;
}

ALWAYS_INLINE unsigned int elapsed(unsigned int ref)
{
    extern volatile unsigned int systemTime;
    return systemTime - ref;
}

ALWAYS_INLINE void Timer::prescaler(unsigned int factor)
{
    timer->PR = factor;
}

ALWAYS_INLINE unsigned int Timer::prescaler() const
{
    return timer->PR;
}

ALWAYS_INLINE void Timer::start()
{
    timer->TCR |= 1;
}

ALWAYS_INLINE void Timer::stop()
{
    timer->TCR &= ~1;
}

ALWAYS_INLINE void Timer::end()
{
    LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << (7 + timerNum));
}

ALWAYS_INLINE void Timer::restart()
{
    timer->TCR = 2;
    timer->TCR = 1;
}

ALWAYS_INLINE void Timer::reset()
{
    timer->TCR |= 2;
    timer->TCR &= ~2;
}

ALWAYS_INLINE unsigned int Timer::value() const
{
    return timer->TC;
}

ALWAYS_INLINE void Timer::value(unsigned int val)
{
    timer->TC = val;
}

ALWAYS_INLINE int Timer::flags() const
{
    return timer->IR;
}

ALWAYS_INLINE void Timer::resetFlags()
{
    timer->IR = 0xff;
}

ALWAYS_INLINE bool Timer::flag(TimerMatch match) const
{
    return timer->IR & (1 << match);
}

ALWAYS_INLINE bool Timer::flag(TimerCapture capture) const
{
    return timer->IR & (16 << capture);
}

ALWAYS_INLINE int Timer::flagMask(TimerMatch match)
{
    return 1 << match;
}

ALWAYS_INLINE int Timer::flagMask(TimerCapture capture)
{
    return 16 << capture;
}

ALWAYS_INLINE void Timer::interrupts()
{
    NVIC_EnableIRQ((IRQn_Type) (TIMER_16_0_IRQn + timerNum));
}

ALWAYS_INLINE void Timer::noInterrupts()
{
    NVIC_DisableIRQ((IRQn_Type) (TIMER_16_0_IRQn + timerNum));
}


ALWAYS_INLINE unsigned int Timer::match(int channel) const
{
    return (&timer->MR0)[channel];
}

ALWAYS_INLINE void Timer::match(int channel, unsigned int value)
{
    (&timer->MR0)[channel] = value;
}

ALWAYS_INLINE unsigned int Timer::capture(int channel) const
{
    return (&timer->CR0)[channel];
}

ALWAYS_INLINE void Timer::pwmEnable(int channel)
{
    timer->PWMC |= 1 << channel;
}

ALWAYS_INLINE void Timer::pwmDisable(int channel)
{
    timer->PWMC &= ~(1 << channel);
}

ALWAYS_INLINE void Timer::matchModePinConfig(int channel, int mode)
{
    int offset = channel << 1;
    timer->EMR &= ~(0x30 << offset);
    timer->EMR |= (mode & 0x30) << offset;
}

ALWAYS_INLINE bool Timer::is32bitTimer(void)
{
    return (bool)(this->timerNum > 1);
}

ALWAYS_INLINE bool Timer::getMatchChannelLevel(int channel)
{
    return (bool)(timer->EMR & (1 << channel));
}

#endif /*sblib_timer_h*/
