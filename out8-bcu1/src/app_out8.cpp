/*
 *  app_out8.cpp - The application for the 8 channel output acting as a Jung 2118
 *
 *  Copyright (c) 2014 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

// TODO PWM handling

#include "app_out8.h"
#include "com_objs.h"
#include <sblib/Timeout.h>
#include <sblib/eib/com_objects.h>
#include <sblib/eib/user_memory.h>

#define PWM_TIMEOUT 10

// state of the application
static unsigned int invertOutputs;
static unsigned int outputs;
static unsigned int oldOutputs;
static unsigned int objectStates;
static unsigned int blockedStates;

static Timeout channel_timeout[8];
static Timeout pwm_timeout;

// internal functions
static void _switch_objects(void);
static void _switch_channels(void);
static void _send_feedback_objects(void);
static void _handle_logic_function(int objno, unsigned int * value);
static void _handle_timed_functions(int objno, unsigned int value);

static const unsigned int  _delayBases[] =
{     1 * MS2TICKS(130)
,     2 * MS2TICKS(130)
,     4 * MS2TICKS(130)
,     8 * MS2TICKS(130)
,    16 * MS2TICKS(130)
,    32 * MS2TICKS(130)
,    64 * MS2TICKS(130)
,   128 * MS2TICKS(130)
,   256 * MS2TICKS(130)
,   512 * MS2TICKS(130)
,  1024 * MS2TICKS(130)
,  2048 * MS2TICKS(130)
,  4096 * MS2TICKS(130)
,  8192 * MS2TICKS(130)
, 16384 * MS2TICKS(130)
, 32768 * MS2TICKS(130)
};

static void _handle_logic_function(int objno, unsigned int * value)
{
	// TODO check function
    unsigned int specialFunc;    // special function number (0: no sf)
    unsigned int specialFuncTyp; // special function type
    unsigned int logicFuncTyp;   // type of logic function ( 1: or, 2: and)
    unsigned int logicState;     // state of logic function
    unsigned int sfOut;          // output belonging to sf
    unsigned int sfMask;         // special function bit mask (1 of 4)

    if (*value)
    {
        objectStates |=  (1 << objno);
    } else
    {
        objectStates &= ~(1 << objno);
    }
    if (objno >= 8)
    {
        /* if a special function is addressed (and changed in most cases),
         * then the "real" object belonging to that sf. has to be evaluated again
         * taking into account the changed logic and blocking states.
         */
        /* determine the output belonging to that sf */
        objno -= 8;
        sfOut  = userEeprom[APP_SPECIAL_CONNECT + (objno >> 1)]
              >> ((objno & 1) * 4) & 0x0F;
        /* get associated object no. and state of that object*/
        if (sfOut)
        {
            if (sfOut > 8) return;
            objno   =  sfOut-1;
            * value = (objectStates >> (sfOut - 1)) & 0x01;
        }
        else return;
        /* do new evaluation of that object */
    }

    /** logic function */
    /* check if we have a special function for this object */
    logicFuncTyp = 0;
    for (specialFunc = 0; specialFunc < 4; specialFunc++)
    {
        sfMask = 1 << specialFunc;
        sfOut  = userEeprom[APP_SPECIAL_FUNC_OBJ_1_2 + (specialFunc >> 1)] >> ((specialFunc & 1) * 4) & 0x0F;
        if (sfOut == (objno + 1))
        {
            /* we have a special function, see which type it is */
            specialFuncTyp = userEeprom[APP_SPECIAL_FUNC_MODE] >> (specialFunc * 2) & 0x03;
            /* get the logic state from the special function object */
            logicState = ((objectStates >> specialFunc) >> 8) & 0x01;
            if (specialFuncTyp == 0)
            {
                /* logic function */
                logicFuncTyp = userEeprom[APP_SPECIAL_LOGIC_MODE] >> (specialFunc * 2) & 0x03;
                if (logicFuncTyp == 1)
                {   // or
                    *value |= logicState;
                }
                if (logicFuncTyp == 2)
                {   // and
                    *value &= logicState;
                }
            }

            if (specialFuncTyp == 1)
            {
                /* blocking function */
                if (((objectStates >> 8) ^ userEeprom[APP_SPECIAL_POLARITY]) & sfMask)
                {   /* start blocking */
                    if (blockedStates & sfMask)
                    {
                        return; // we are blocked, do nothing
                    }
                    blockedStates |= sfMask;
                    *value = (userEeprom[APP_SPECIAL_FUNCTION1 + (specialFunc>>1)])>>((specialFunc&1)*4)&0x03;
                    if (*value == 0)
                    {
                        return;
                    }
                    if (*value == 1)
                    {
                        outputs &= ~(1<<objno);
                    }
                    if (*value == 2) {
                        outputs |= (1<<objno);
                    }
                    objectValues.outputs[objno] = (outputs >> objno) & 0x1 ;
                    objectWrite(objno, (unsigned int) objectValues.outputs[objno]);
                    _switch_objects();
                    return;
                } else {
                    /* end blocking */
                    if (blockedStates & sfMask )
                    {   // we have to unblock
                        blockedStates &= ~sfMask;
                        /* action at end of blocking, 0: nothing, 1: off, 2: on */
                        *value = (userEeprom[APP_SPECIAL_FUNCTION1 + (specialFunc>>1)])>>((specialFunc&1)*4+2)&0x03;
                        if (*value == 0)
                        {
                            return;
                        }
                        (*value)--;
                        /* we are unblocked, continue as normal */
                    }
                }
            }
        }
    }
}

static void _handle_timed_functions(int objno, unsigned int value)
{
	// TODO check function !
    unsigned int mask             = 1 << objno;
    // Set some variables to make next commands better readable_handle_timed_functions
    unsigned int timerCfg       = userEeprom[APP_DELAY_ACTIVE] & mask;
    unsigned int timerOffFactor = userEeprom[APP_DELAY_FACTOR_OFF + objno];
    unsigned int timerOnFactor  = userEeprom[APP_DELAY_FACTOR_ON  + objno];

    // Get configured delay base
    unsigned int delayBaseIdx = userEeprom[APP_DELAY_BASE + ((objno + 1) >> 1)];
    unsigned int delayBase;

    if ((objno & 0x01) == 0x00)
        delayBaseIdx >>= 4;
    delayBaseIdx      &= 0x0F;
    delayBase          = _delayBases[delayBaseIdx];

    if (!timerCfg)
    {
        // Check if a delay is configured for falling edge
        if ( (outputs & mask) && !value && timerOffFactor)
        {
        	channel_timeout[objno].start(delayBase * timerOffFactor);
        }
        // Check if a delay is configured for raising edge
        if (!(outputs & mask) && value && timerOnFactor)
        {
        	channel_timeout[objno].start(delayBase * timerOnFactor);
        }
    } else
    {
        if (!timerOnFactor)
        {
            // Check for a timer function without delay factor for raising edge
            if ( !(outputs & mask) && value)
            {
                outputs       |= mask;
            	channel_timeout[objno].start(delayBase * timerOffFactor);
            }
        }
        else
        {
            // Check for a timer function with delay factor for on
            if ( !(outputs & mask) && value)
            {
            	channel_timeout[objno].start(delayBase * timerOnFactor);
            }

            // Check for delay factor for off
            if ((outputs & mask) && !value)
            {
            	channel_timeout[objno].start(delayBase * timerOffFactor);
            }
        }
        // check how to handle off telegram while in timer modus
        if ((outputs & mask) && !value)
        {
            // only switch off if on APP_DELAY_ACTION the value is equal zero
            if (! (userEeprom[APP_DELAY_ACTION] & mask))
            {
                if (channel_timeout[objno].started())
                {
                	channel_timeout[objno].stop();
                    outputs       &= ~mask;
                }
            }
        }
    }
}

void objectUpdated(int objno)
{
    unsigned int value;
    // TODO check if the information that a channel shall be switch needs to be passed to the caller?

    // get value of object (0=off, 1=on)
    value = objectRead(objno);

    // handle the logic functions for this channel
    _handle_logic_function(objno, & value);

    // check if we have a delayed action for this object, only Outputs
    if(objno < COMOBJ_SPECIAL1)
    {
        _handle_timed_functions(objno, value);
    }

    if(channel_timeout[objno].stopped())
    {
        if (value == 0x01)
        {
            outputs |=   1 << objno;
        }
        else
        {
            outputs &= ~(1 << objno);
        }
    }
}

void checkTimeouts(void)
{
    unsigned int needToSwitch = 0;
    unsigned int objno;

#ifdef HAND
    // manual Operation is enabled
    // check one button every app_loop() passing through
    if (handActuationCounter <= OBJ_OUT7  &&  checkHandActuation(handActuationCounter))
    {
        needToSwitch=1;
    }
    handActuationCounter++;     //count to 255 for debounce buttons, ca. 30ms
#endif

    // check if we can enable PWM
    if(pwm_timeout.expired())
    {
        //sb_pwm_setup_duty_cycle(SB_PWM_P1_10, 33);
    }

    for (objno = 0; objno < COMOBJ_SPECIAL1; ++objno) {
        if(channel_timeout[objno].expired())
        {
            unsigned int obj_value;
            outputs                 ^= 1 << objno;
            obj_value                = (outputs >> objno) & 0x1;
            _handle_timed_functions(objno, obj_value);
            needToSwitch             = 1;
        }
    }

    if(needToSwitch)  _switch_objects();
}

static void _send_feedback_objects(void)
{
    unsigned int changed = outputs ^ oldOutputs;

    if(changed)
    {   // at least one output has changed, requires sending a feedback objects
        unsigned int value = outputs ^ userEeprom[APP_REPORT_BACK_INVERT];
        unsigned int i;
        unsigned int mask = 0x01;
        for (i = 0; i < 8; i++)
        {
            if(changed & mask)
            {   // this output pin has changed. set the object value
            	objectValues.feedback[i] = (value & mask) ? 1 : 0;
                // and request the transmission of the feedback object
                objectWrite(COMOBJ_FEEDBACK1 + i, (unsigned int) objectValues.feedback[i]);
            }
            mask <<= 1;
        }
   }
}

static void _switch_objects(void)
{
    _send_feedback_objects();
    _switch_channels();
}

static void _switch_channels(void)
{
	int i;
	int mask = 0x01;
	int value = outputs ^ invertOutputs;

    if((outputs ^ oldOutputs) & outputs)
    { // at least one port will be switched from 0 to 1 -> need to disable the PWM
        //sb_pwm_setup_duty_cycle(SB_PWM_P1_10, 100); // disable the PWM
    	pwm_timeout.start(PWM_TIMEOUT);
    }
    for(i = 0; i < NO_OF_CHANNELS; i++, mask <<= 1)
    {
    	digitalWrite(outputPins[i], value & mask);
    }
    oldOutputs = outputs;
}

void initApplication(void)
{
    unsigned int i;
    unsigned int initialChannelAction;

    //sb_pwm_setup_period(SB_PWM_10KHZ);
    //sb_pwm_setup_duty_cycle(SB_PWM_P1_10, 100); // set the initial duty cycle to 100%
    outputs              = userEeprom[APP_PIN_STATE_MEMORY]; // load from eeprom
    oldOutputs           = ~outputs;                 // force a "toggle" on each output
    invertOutputs        = userEeprom[APP_CLOSER_MODE];
    initialChannelAction = userEeprom[APP_RESTORE_AFTER_PL_HI] << 8
                                   | userEeprom[APP_RESTORE_AFTER_PL_LO];
    for (i=0; i < 8; i++) {
        unsigned int temp = (initialChannelAction >> (i * 2)) & 0x03;
        if      (temp == 0x01)
        {   // open contact
            outputs &= ~(1 << i);
        }
        else if (temp == 0x02)
        {   // close contact
            outputs |=  (1 << i);
        }
        // Send status of every object to bus on power on
    	objectValues.outputs[i] = (outputs >> i) & 0x1;
        // and request the transmission of the feedback object
        objectWrite(i, (unsigned int) objectValues.outputs[i]);
    }
    _switch_channels();
}
