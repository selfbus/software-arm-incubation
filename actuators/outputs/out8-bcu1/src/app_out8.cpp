/*
 *  app_out8.cpp - The application for the 8 channel output acting as a Jung 2118
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_out8.h"
#include "com_objs.h"

#ifndef BI_STABLE
#   include "outputs.h"
#else
#   include "outputsBiStable.h"
#endif

#ifdef HAND_ACTUATION
#   include "hand_actuation.h"
#endif

#include <sblib/timeout.h>
#include <sblib/eib/com_objects.h>
#include <sblib/eib/user_memory.h>

// state of the application
static Timeout channel_timeout[NO_OF_CHANNELS];

// internal functions
static void          _switchObjects(void);
static void          _sendFeedbackObjects(bool forceSendFeedback = false);
static void          _handle_logic_function(int objno, unsigned int value);
static void          _handle_timed_functions(const int objno, const unsigned int value, const unsigned int ignoreoutputstate = 0);


#define MSBASE 130
static const unsigned int  _delayBases[] =
{     1 * MS2TICKS(MSBASE)
,     2 * MS2TICKS(MSBASE)
,     4 * MS2TICKS(MSBASE)
,     8 * MS2TICKS(MSBASE)
,    16 * MS2TICKS(MSBASE)
,    32 * MS2TICKS(MSBASE)
,    64 * MS2TICKS(MSBASE)
,   128 * MS2TICKS(MSBASE)
,   256 * MS2TICKS(MSBASE)
,   512 * MS2TICKS(MSBASE)
,  1024 * MS2TICKS(MSBASE)
,  2048 * MS2TICKS(MSBASE)
,  4096 * MS2TICKS(MSBASE)
,  8192 * MS2TICKS(MSBASE)
, 16384 * MS2TICKS(MSBASE)
, 32768 * MS2TICKS(MSBASE)
};

enum specialFunctionType {sftUnknown = -1, sftLogic = 0, sftBlocking = 1, sftConstrainedLead = 2}; // Verknuepfungsobjekt, Sperrobjekt, Zwangsstellungsobjekt
enum logicType {ltUnknown = -1, ltOR = 1, ltAND = 2, ltAND_RECIRC = 3};
enum blockType {blUnknown = -1, blNoAction = 0, blDisable = 1, blEnable = 2};

static void _handle_logic_function(int objno, unsigned int value)
{
    unsigned int lockPolarity = userEeprom[APP_SPECIAL_POLARITY]; // polarity of the lock object
    unsigned int specialFunc;           // special function number (0: no special function)
    specialFunctionType specialFuncTyp; // special function type
    logicType logicFuncTyp;             // type of logic function ( 1: or, 2: and)
    bool startBlocking;
    blockType blockTyp;
    unsigned int logicState;            // state of logic function
             int specialFuncOutput;     // output the special function is belonging to

    static unsigned int specialObjStates [COMOBJ_SPECIAL4 - COMOBJ_SPECIAL1 + 1];

    // output objects are from 0->7 & special functions are from 8->11, otherwise return
    if ((objno < COMOBJ_INPUT1) || (objno > COMOBJ_SPECIAL4))
        return;

    // special functions are from 8->11, determine the output belonging to the special function
    if ((objno >= COMOBJ_SPECIAL1) && (objno <= COMOBJ_SPECIAL4))
    {
        objno -= COMOBJ_SPECIAL1;
        specialFuncOutput  = userEeprom[APP_SPECIAL_CONNECT + (objno >> 1)] >> ((objno & 1) * 4) & 0x0F;

        // output for special function in range 1..8, otherwise return
        if ((specialFuncOutput <= COMOBJ_INPUT1) || (specialFuncOutput > (COMOBJ_INPUT1 + NO_OF_CHANNELS)))
            return;

        /* if a special function is addressed (and changed in most cases),
         * then the "real" object belonging to that special function. has to be evaluated
         * again taking into account the changed logic and blocking states.
         */

        // specialFuncOutput range 1..8, object number range for outputs is 0..7
        objno =  specialFuncOutput - 1;

        // get the current value of the object from the internal state
        // this will reflect the state after the timeout handling!
        value = objectRead(objno);
    }

    // check if we have a special function for this object
    for (specialFunc = 0; specialFunc < 4; specialFunc++)
    {
        specialFuncOutput  = userEeprom[APP_SPECIAL_FUNC_OBJ_1_2 + (specialFunc >> 1)] >> ((specialFunc & 1) * 4) & 0x0F;

        if (specialFuncOutput != (objno + 1))
            continue;

        // we have a special function, see which type it is
        specialFuncTyp = specialFunctionType (userEeprom[APP_SPECIAL_FUNC_MODE] >> (specialFunc * 2) & 0x03);
        switch (specialFuncTyp)
        {
        case sftLogic : // logic function (OR/AND/AND with recirculation
            // get the logic state for the special function object
            logicState = objectRead(COMOBJ_SPECIAL1 + specialFunc);
            logicFuncTyp = logicType (userEeprom[APP_SPECIAL_LOGIC_MODE] >> (specialFunc * 2) & 0x03);
            switch (logicFuncTyp)
            {
            case ltOR : // or
                value |= logicState;
                break;
            case ltAND : // and
                value &= logicState;
                break;
            case ltAND_RECIRC : // and with recirculation
                value &= logicState;
                if (specialObjStates [specialFunc] && !logicState)
                    // if the logic part of the and connection has been
                    // cleared -> clear also the real object value
                    objectUpdate(objno, value);
                specialObjStates [specialFunc] = logicState;
                break;
            }
            relays.updateChannel(objno, value);
            break; // case sftLogic

        case sftBlocking: // blocking function
            startBlocking = (objectRead(COMOBJ_SPECIAL1 + specialFunc) ^ (lockPolarity >> objno)) & 0x01;
            if (startBlocking)
            {
                // action at start of blocking
                blockTyp = blockType ((userEeprom[APP_SPECIAL_FUNCTION1 + (specialFunc>>1)])>>((specialFunc&1)*4)&0x03);
            }
            else
            {
                // action at end of blocking
                blockTyp = blockType ((userEeprom[APP_SPECIAL_FUNCTION1 + (specialFunc>>1)])>>((specialFunc&1)*4+2)&0x03);
                // end blocking, we have to unblock relays
                if (relays.blocked(objno))
                    relays.clearBlocked(objno);
            }

            if (!relays.blocked (objno))
            {
                switch (blockTyp)
                {
                case blNoAction : // no action
                    break;
                case blDisable : // disable the output
                    relays.updateChannel(objno, false);
                    break;
                case blEnable : // enable the output
                    relays.updateChannel(objno, true);
                    break;
                default:
                    break;
                }
            }

            //finally set relays blocked in case of blocking start
            if (startBlocking)
                relays.setBlocked(objno);
            break; // case sftBlocking

        case sftConstrainedLead: // constrained lead
            value = objectRead (COMOBJ_SPECIAL1 + specialFunc);
            if (value & 0b10)
            {   /* constrained lead is active for this channel
                ** set the value of the special com object as output state
                */
                relays.updateChannel(objno, value & 0b01);
            }
            else if (specialObjStates [specialFunc] & 0b10)
            {   // the contrained lead was just deactivated
                // restore the output based on the object state
                relays.updateChannel(objno, objectRead (objno));
            }
            specialObjStates [specialFunc] = value;
            break; // case sftConstrainedLead
        }

        break; // leave for, we handled the special function & only one function per output is allowed
    } // for
}

static void _handle_timed_functions(const int objno, const unsigned int value, const unsigned int ignoreoutputstate)
{
    // check that objno is in a valid range
    if ((objno < COMOBJ_INPUT1) || (objno >= (COMOBJ_INPUT1+NO_OF_CHANNELS)))
        return;

    unsigned int mask           = 1 << objno;
    // Set some variables to make next commands better readable
    unsigned int timerCfg       = userEeprom[APP_DELAY_ACTIVE] & mask;
    unsigned int timerOffFactor = userEeprom[APP_DELAY_FACTOR_OFF + objno];
    unsigned int timerOnFactor  = userEeprom[APP_DELAY_FACTOR_ON  + objno];
    unsigned int outputState    = relays.channel(objno);
    // Get configured delay base
    unsigned int delayBaseIdx   = userEeprom[APP_DELAY_BASE + ((objno + 1) >> 1)];
    unsigned int delayBase;

    // ignoreoutputstate will only be set for initialization/start-up
    if (ignoreoutputstate)
        outputState = !value;

    if ((objno & 0x01) == 0x00)
        delayBaseIdx >>= 4;
    delayBaseIdx      &= 0x0F;
    delayBase          = _delayBases[delayBaseIdx];

    if (!timerCfg) // this is the on/off delay mode
    {   // Check if a delay is configured for falling edge
        if (outputState && !value && timerOffFactor)
        {
            channel_timeout[objno].start(delayBase * timerOffFactor);
        }
        // Check if a delay is configured for raising edge
        else if (!outputState && value && timerOnFactor)
        {
            channel_timeout[objno].start(delayBase * timerOnFactor);
        }
        // for initialization of channels without a on and/or off delay
        else if (!timerOffFactor && !timerOnFactor && ignoreoutputstate)
        {
            channel_timeout[objno].start(1);
        }
    }
    else // this is the timed function mode
    {
        if (!timerOnFactor) // no delay for  on -> switch on the output
        {
            // Check for a timer function without delay factor for raising edge
            if ( !outputState && value)
            {
                relays.setChannel(objno);
                objectWrite(objno, (unsigned int) 1);
                channel_timeout[objno].start (delayBase * timerOffFactor);
            }
        }
        else
        {
            // Check for a timer function with delay factor for on
            if ( !outputState && value)
            {
                channel_timeout[objno].start (delayBase * timerOnFactor);
            }

            // Check for delay factor for off
            if (outputState && value)
            {   // once the output is ON start the OFF delay
                channel_timeout[objno].start(delayBase * timerOffFactor);
            }
        }
        // check how to handle off telegram while in timer modus
        if (outputState && !value)
        {   // only switch off if APP_DELAY_ACTION the value is equal zero
            if (! (userEeprom[APP_DELAY_ACTION] & mask))
            {
                relays.clearChannel(objno);
                objectWrite(objno, (unsigned int) 0);
            }
        }
    }
}

void objectUpdated(int objno)
{
    unsigned int value;

    // get value of object (0=off, 1=on)
    value = objectRead(objno);
    // check if we have a delayed action for this object, only Outputs
    if(objno < COMOBJ_SPECIAL1)
    {
        _handle_timed_functions(objno, value);
        if(channel_timeout[objno].stopped())
        {
            relays.updateChannel(objno, value);
            objectWrite(objno, value);
        }
    }

    // handle the logic functions for this channel
    _handle_logic_function (objno, value);

    if (relays.pendingChanges())
        _switchObjects();
}

void checkTimeouts(void)
{
    unsigned int objno;

#ifdef HAND_ACTUATION
    int handStatus = handAct.check();
    if (handStatus != HandActuation::NO_ACTUATION)
    {
        unsigned int number = handStatus & 0xFF;
        if (handStatus & HandActuation::BUTTON_PRESSED)
            relays.toggleChannel(number);
    }
#endif

    // check if we can enable PWM
    relays.checkPWM();
    for (objno = 0; objno < COMOBJ_SPECIAL1; ++objno)
    {
        if (channel_timeout[objno].expired ())
        {
            unsigned int obj_value = relays.toggleChannel(objno);
            objectWrite(objno, obj_value);
            _handle_timed_functions(objno, obj_value);
            _handle_logic_function(objno, obj_value);
        }
    }

    if(relays.pendingChanges ())  _switchObjects();
}

static void _sendFeedbackObjects(bool forcesendFeedbackObjects)
{
    unsigned int changed = relays.pendingChanges ();

    if (changed || forcesendFeedbackObjects)
    {   // at least one output has changed,
        // -> update the corresponding feedback objects
        unsigned int i;
        unsigned int mask = 0x01;
        unsigned int invert = userEeprom[APP_REPORT_BACK_INVERT];
        for (i = 0; i < 8; i++)
        {
            if ((changed & mask) || forcesendFeedbackObjects)
            {   // update feedback object
            	unsigned int value = relays.channel(i);
            	if (invert & mask)
            		value ^= 0x01;
                objectWrite(COMOBJ_FEEDBACK1 + i, value);
            }
            mask <<= 1;
        }
   }
}

static void _switchObjects(void)
{
    _sendFeedbackObjects();
    relays.updateOutputs();
}

void initApplication(int lastRelayState)
{
    unsigned int i;
    unsigned int initialChannelActions;
    int newRelaystate;
    Outputs::State initialOutputState[NO_OF_CHANNELS];

    delay(1000); // delay((rand() % 5)* 1000); // FIXME delay the app start by some random seconds,
                                               // so not all out8-apps will return the same time the bus returns.
                                               // try to read open/floating analog input IO12 /AD5 to get a random number

    // read & combine initialChannelAction's low & high byte from userEeprom
    // 2 bits for each channel: 0x00=LAST_STATE, 0x01=OPEN, 0x02=CLOSED, e.g. initialChannelAction: 0xAAAA Ch1-8 closed; 0x5555 Ch1-8 open; 0x0000 Ch1-8 last state
    initialChannelActions = (userEeprom[APP_RESTORE_AFTER_PL_HI] << 8) | userEeprom[APP_RESTORE_AFTER_PL_LO];

    newRelaystate = 0x00;
    for (i=0; i < NO_OF_CHANNELS; i++) {
        unsigned int ChannelAction = (initialChannelActions >> (i * 2)) & 0x03;
        if (ChannelAction == 0x01)
            initialOutputState[i] = Outputs::OPEN;
        else if (ChannelAction == 0x02)
            initialOutputState[i] = Outputs::CLOSED;
        else
        {
            initialOutputState[i] = Outputs::OPEN;
#ifdef BUSFAIL
            if ((lastRelayState >> i) & 0x01)
                initialOutputState[i] = Outputs::CLOSED;
#endif
        }
        // create the RelaysBeginState according to the requested initial outputs states
        if (initialOutputState[i] == Outputs::CLOSED)
            newRelaystate |= 1 << i;
    }

    // set the initial relays state
    relays.begin(newRelaystate, userEeprom[APP_CLOSER_MODE]);

    // set relays according to delayed on/off or timed configuration
    for (i=COMOBJ_INPUT1; i < (COMOBJ_SPECIAL1); i++)
    {
        if (i >= sizeof(initialOutputState)/sizeof(initialOutputState[0])) // check initialOutputState array bounds
            break;
        unsigned int value = (initialOutputState[i] == Outputs::CLOSED);
        _handle_timed_functions(i, value, true); // check for timed functions for the channel
    }

    for (i=COMOBJ_SPECIAL1; i <= COMOBJ_SPECIAL4; i++)
    {
        objectUpdate(i, (unsigned int) 0); // set all logic objects to false
        _handle_logic_function (i, 0); // handle the logic functions for the channel
        // FIXME handle logic function on start-up correctly
        // maybe read the real-value from the bus (sendGroupReadTelegram)
        // _handle_logic_function (i, valuefrombus); // handle the logic functions for the channel
    }

}

int  getRelaysState()
{
    int state = 0x00;
    for (int i=0; i < NO_OF_CHANNELS; i++)
    {
        if (relays.channel(i))
        {
            state |=  (1 << i);
        }
    }
    return state;
}
