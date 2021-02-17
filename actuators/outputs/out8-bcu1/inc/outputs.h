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

#ifndef _outputs_h_
#define _outputs_h_ 1

#include <sblib/timeout.h>
#include <sblib/timer.h>
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>
#include "config.h"

#ifdef HAND_ACTUATION
#   include "hand_actuation.h"
#endif

#define PWM_TIMEOUT 20 // ms
#define PWM_PERIOD  85 // 1.2kHz
#define PWM_DUTY    22 // 0.25 duty

/*
 * order code relay Omron G5Q-1A-EU DC24
 *    1:     1 pole
 *    A:     SPST-NO
 *   EU:     high-capacity
 * DC24:     24V DC rated coil voltage
 *
 * according to the datasheet relay must operate @max.75% of rated coil voltage => ~18V
 * PWM operation: first 100ms 100% of rated voltage (24V) then 30% (7.2V) for holding
 *
 * XXX: above PWM settings don't fit the datasheet spec.
 */

class Outputs
{
public:
    enum State : byte {OPEN = 0x00, CLOSED = 0x01, LAST_STATE = 0xFF};
    Outputs() : _channelcount(0)
              , _relayState(0)
              , _prevRelayState(0)
              , _inverted(0)
              , _blocked(0)
              {};

    void begin(unsigned int initial, unsigned int inverted, unsigned int channelcount, const int* Pins, const unsigned int pinCount);
    unsigned int pendingChanges(void);
    unsigned int channel(unsigned int channel);
    void updateChannel(unsigned int channel, unsigned int value);
    void setChannel(unsigned int channel);
    void clearChannel(unsigned int channel);
    unsigned int toggleChannel(unsigned int channel);
    unsigned int blocked(unsigned int channel);
    void setBlocked(unsigned int channel);
    void clearBlocked(unsigned int channel);
    virtual void checkPWM(void);
    virtual unsigned int updateOutput(unsigned int channel);      // returns true in case a switching action was started which drained the bus
    virtual void updateOutputs(unsigned int delayms = 0);
    void setOutputs(void);
    void clrOutputs(void);
    unsigned int channelCount();
    unsigned int outputCount();

#ifdef HAND_ACTUATION
    void setHandActuation(HandActuation* hand);
#endif

#ifdef ZERO_DETECT
    void zeroDetectHandler(void);
#endif

protected:
    unsigned int _channelcount;
    unsigned int _relayState;
    unsigned int _prevRelayState;
    unsigned int _inverted;
    unsigned int _modified;
    unsigned int _blocked;
    Timeout      _pwm_timeout;
    unsigned int _port_0_set;
    unsigned int _port_2_set;
    unsigned int _port_0_clr;
    unsigned int _port_2_clr;
    unsigned int _pinCount;
    unsigned int *_outputPins;
#ifdef HAND_ACTUATION
    HandActuation *_handAct;
#endif

#ifdef ZERO_DETECT
    unsigned int _state;
#endif
};

#ifndef BI_STABLE
extern Outputs relays;
#endif

ALWAYS_INLINE unsigned int Outputs::pendingChanges(void)
{
    return _relayState ^ (_prevRelayState & ((1 << _channelcount) -1));
}

ALWAYS_INLINE unsigned int Outputs::channel(unsigned int channel)
{
    return _relayState & (1 << channel) ? 1 : 0;
}

ALWAYS_INLINE void Outputs::updateChannel(unsigned int channel, unsigned int value)
{
    if (value) setChannel(channel);
    else       clearChannel(channel);
}

ALWAYS_INLINE void Outputs::setChannel(unsigned int channel)
{
    unsigned int mask = 1 << channel;
    if (! (_blocked & mask))
       _relayState |=  mask;
}

ALWAYS_INLINE void Outputs::clearChannel(unsigned int channel)
{
    unsigned int mask = 1 << channel;
    if (! (_blocked & mask))
        _relayState &= ~mask;
}

ALWAYS_INLINE unsigned int Outputs::toggleChannel(unsigned int channel)
{
    if (this->channel(channel))
    {
        clearChannel(channel);
        return 0;
    }
    else
    {
        setChannel(channel);
        return 1;
    }
}

ALWAYS_INLINE unsigned int Outputs::blocked(unsigned int channel)
{
    return _blocked & (1 << channel);
}

ALWAYS_INLINE void Outputs::setBlocked(unsigned int channel)
{
    _blocked |= (1 << channel);
}

ALWAYS_INLINE void Outputs::clearBlocked(unsigned int channel)
{
    _blocked &= ~(1 << channel);
}

ALWAYS_INLINE void Outputs::checkPWM(void)
{
    if(_pwm_timeout.expired())
    {
        timer16_0.match(MAT2, PWM_DUTY);
        //digitalWrite(PIO1_4, 0);
    }
}

#ifdef ZERO_DETECT
ALWAYS_INLINE void Outputs::zeroDetectHandler(void)
{
	if (_state = 1)
	{
	    //digitalWrite(PIN_INFO, ! digitalRead(PIN_INFO));	// Info LED
		timer32_0.start();
		_state = 0;
	}
}
#endif /* define ZERO_DETECT */

ALWAYS_INLINE void Outputs::setOutputs(void)
{
	if (_port_0_set || _port_2_set)
	{   // at least one port will be switched on
        timer16_0.match(MAT2, PWM_PERIOD);// disable the PWM
        _pwm_timeout.start(PWM_TIMEOUT);
	}
#ifdef __LPC11XX__
   	gpioPorts[0]->MASKED_ACCESS[_port_0_set] = 0xFFFF;
   	gpioPorts[2]->MASKED_ACCESS[_port_2_set] = 0xFFFF;
#else
   	LPC_GPIO->SET[0] = _port_0_set;
   	LPC_GPIO->SET[1] = _port_2_set;
#endif
    _port_0_set = _port_2_set = 0;
}

ALWAYS_INLINE void Outputs::clrOutputs(void)
{
#ifdef __LPC11XX__
   	gpioPorts[0]->MASKED_ACCESS[_port_0_clr] = 0x0000;
   	gpioPorts[2]->MASKED_ACCESS[_port_2_clr] = 0x0000;
#else
   	LPC_GPIO->CLR[0] = _port_0_clr;
   	LPC_GPIO->CLR[1] = _port_2_clr;
#endif
    _port_0_clr = _port_2_clr = 0;
}

#endif
