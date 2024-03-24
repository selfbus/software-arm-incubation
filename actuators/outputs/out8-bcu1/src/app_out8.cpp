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
#include <sblib/timeout.h>
#include <sblib/eib/com_objects.h>


#ifndef BI_STABLE
#   include "outputs.h"
#else
#   include "outputsBiStable.h"
#endif

#ifdef HAND_ACTUATION
#   include "hand_actuation.h"
#endif


typedef struct ChannelTimeOutTimer {
    Timeout On;
    Timeout Off;
} ChannelTimeOutTimer;

enum specialFunctionType {sftUnknown = -1, sftLogic = 0, sftBlocking = 1, sftConstrainedLead = 2}; // Verknuepfungsobjekt, Sperrobjekt, Zwangsstellungsobjekt
enum logicType {ltUnknown = 0, ltOR = 1, ltAND = 2, ltAND_RECIRC = 3}; // logic OR, AND, AND with recirculation (UND mit Rueckfuehrung)
enum logicResult {lrUnchanged = 0, lrSetOn = 1, lrSetOff = 2};
enum blockType {blUnknown = -1, blNoAction = 0, blDisable = 1, blEnable = 2};
enum timedFunctionState {tfsUnknown = 0x80, tfsDisabled = 0, tfsOnDelayed = 1, tfsOffDelayed = 2};

BCU1 bcu;

// state of the application
static ChannelTimeOutTimer channel_timeout[NO_OF_CHANNELS];

#ifdef HAND_ACTUATION
    HandActuation handAct = HandActuation(&handPins[0], NO_OF_HAND_PINS, READBACK_PIN, BLINK_TIME);
#endif

// internal functions
static void          _switchObjects(unsigned int delayms = 0);
static void          _sendFeedbackObjects(bool forceSendFeedback = false);
static void          _handle_logic_function(int objno, unsigned int value);
static unsigned int  _handle_timed_functions(const int objno, const unsigned int value);
// static logicResult   _init_logic_function(const int objno, unsigned int value);
// static unsigned int  _init_timed_functions(const int objno, const unsigned int value);


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



typedef struct TimerConfig {
    unsigned int timerMode;
    unsigned int timerDelayAction;
    unsigned int timerOnFactor;
    unsigned int timerOffFactor;
    unsigned int delayMs;
} TimerConfig;

typedef struct SpecialFunctionConfig
{
    specialFunctionType Mode;  // mode of the special function
    int specialFuncNumber;     // number of the special function (0..3)
    int specialFuncOutput;     // output a special function is belonging to (0..7)
    logicType logicFuncTyp;
    blockType blockTypeStart;
    blockType blockTypeEnd;
    unsigned int lockPolarity; // polarity of the lock object for sftBlocking mode
} SpecialFunctionConfig;

SpecialFunctionConfig getSpecialFunctionConfig(const int objno)
{
    SpecialFunctionConfig sfcfg;
    int outputobjno;
    int outputobjnoSearching;
    unsigned int sfobjno;
    unsigned int specialFunc;           // special function number

    sfcfg.Mode = sftUnknown;
    sfcfg.specialFuncNumber = -1;
    sfcfg.specialFuncOutput = -1;
    sfcfg.logicFuncTyp = ltUnknown;
    sfcfg.blockTypeStart = blUnknown;
    sfcfg.blockTypeEnd = blUnknown;
    sfcfg.lockPolarity = 0;

    // output objects are from 0->7 & special functions are from 8->11, otherwise return
    if ((objno < COMOBJ_INPUT1) || (objno > COMOBJ_SPECIAL4))
    {
        sfcfg.Mode = sftUnknown;
        return sfcfg;
    }

    // special functions are from 8->11, determine the output belonging to the special function
    if ((objno >= COMOBJ_SPECIAL1) && (objno <= COMOBJ_SPECIAL4))
    {
        sfobjno = objno - COMOBJ_SPECIAL1;
        // "APP_SPECIAL_CONNECT" range 1..8, object number range for outputs is 0..7, thats why -1
        outputobjnoSearching  = ((*(bcu.userEeprom))[APP_SPECIAL_CONNECT + (sfobjno >> 1)] >> ((sfobjno & 1) * 4) & 0x0F) - 1;
        // output for special function in range 0..7, otherwise return
        if ((outputobjnoSearching < COMOBJ_INPUT1) || (outputobjnoSearching > (COMOBJ_INPUT1 + NO_OF_CHANNELS)))
        {
            sfcfg.Mode = sftUnknown;
            return sfcfg;
        }
    }
    else
    {
        outputobjnoSearching = objno;
    }

    // check if we have a special function for outputobjnoSearching
    for (specialFunc = 0; specialFunc < 4; specialFunc++)
    {
        // "APP_SPECIAL_FUNC_OBJ_1_2" range 1..8, object number range for outputs is 0..7, thats why -1
        outputobjno  = ((*(bcu.userEeprom))[APP_SPECIAL_FUNC_OBJ_1_2 + (specialFunc >> 1)] >> ((specialFunc & 1) * 4) & 0x0F) - 1;
        if (outputobjno != outputobjnoSearching)
            continue;

        sfcfg.specialFuncOutput = outputobjno;
        sfcfg.specialFuncNumber = specialFunc;
        // we have a special function, see which type it is
        sfcfg.Mode = specialFunctionType ((*(bcu.userEeprom))[APP_SPECIAL_FUNC_MODE] >> (sfcfg.specialFuncNumber * 2) & 0x03);

        switch (sfcfg.Mode)
        {
        case sftLogic : // logic function (OR/AND/AND with recirculation;
            sfcfg.logicFuncTyp = logicType ((*(bcu.userEeprom))[APP_SPECIAL_LOGIC_MODE] >> (sfcfg.specialFuncNumber * 2) & 0x03);
            break;
        case sftBlocking: // blocking function
            sfcfg.blockTypeStart = blockType (((*(bcu.userEeprom))[APP_SPECIAL_FUNCTION1 + (sfcfg.specialFuncNumber>>1)])>>((sfcfg.specialFuncNumber&1)*4)&0x03);
            sfcfg.blockTypeEnd = blockType (((*(bcu.userEeprom))[APP_SPECIAL_FUNCTION1 + (sfcfg.specialFuncNumber>>1)])>>((sfcfg.specialFuncNumber&1)*4+2)&0x03);
            sfcfg.lockPolarity = ((*(bcu.userEeprom))[APP_SPECIAL_POLARITY] >> sfcfg.specialFuncNumber) & 0x01; // polarity of the lock object
            break;
        case sftConstrainedLead: // constrained lead
            break;
        default:
            break;
        }
        break; // for
    }
    return sfcfg;
}

static TimerConfig getTimerCfg(const int objno)
{
    TimerConfig timercfg;

    unsigned int mask         = 1 << objno;
    unsigned int delayBaseIdx = (*(bcu.userEeprom))[APP_DELAY_BASE + ((objno + 1) >> 1)];
    timercfg.timerMode        = (*(bcu.userEeprom))[APP_DELAY_ACTIVE] & mask;
    timercfg.timerDelayAction = (*(bcu.userEeprom))[APP_DELAY_ACTION] & mask;
    timercfg.timerOffFactor   = (*(bcu.userEeprom))[APP_DELAY_FACTOR_OFF + objno];
    timercfg.timerOnFactor    = (*(bcu.userEeprom))[APP_DELAY_FACTOR_ON  + objno];

    if ((objno & 0x01) == 0x00)
        delayBaseIdx >>= 4; // why this shift for channel 2? but it seems to work...
    delayBaseIdx      &= 0x0F;
    timercfg.delayMs = _delayBases[delayBaseIdx];

    return timercfg;
}

static void _handle_logic_function(int objno, unsigned int value)
{
    // FIXME debug the logic handling! untested right now
    bool startBlocking;         // true if a blocking is started
    bool endBlocking;           // true if a blocking was ended
    blockType blockTyp;         // holds the type of blocking
    unsigned int logicState;    // state of logic function

    SpecialFunctionConfig sfcfg = getSpecialFunctionConfig(objno);

    switch (sfcfg.Mode)
    {
    case sftUnknown :
         break; // sftUnknown

    case sftLogic : // logic function (OR/AND/AND with recirculation
        if ((objno >= COMOBJ_SPECIAL1) && (objno <= COMOBJ_SPECIAL4))// need the value of the actual input 0-7
            value = bcu.comObjects->objectRead(sfcfg.specialFuncOutput);

        // get the logic state for the special function object
        logicState = bcu.comObjects->objectRead(COMOBJ_SPECIAL1 + sfcfg.specialFuncNumber);
        switch (sfcfg.logicFuncTyp)
        {
        case ltOR : // or
            value |= logicState;
            break;
        case ltAND : // and
            value &= logicState;
            break;
        case ltAND_RECIRC : // and with recirculation
             // UND mit Rückführung:
             // Verknüpfungs-Objekt = "0" Ausgang ist immer "0" (logisch UND).
             // In diesem Fall wird durch die Rückführung des Ausgangs auf das Schalten-Objekt dieses beim Setzen wieder zurückgesetzt.
             // Erst, wenn das Verknüpfungs-Objekt = "1" ist, kann durch eine neu empfangene "1" am Schalten-Objekt der
             // Ausgang den logischen Zustand "1" annehmen.
            if (!logicState)
            {
                // if the logic part of the and connection has been
                // cleared -> clear also the real object value
                bcu.comObjects->objectSetValue(sfcfg.specialFuncOutput, false);
                value = false;
                channel_timeout[sfcfg.specialFuncOutput].On.stop();
                channel_timeout[sfcfg.specialFuncOutput].Off.stop();
            }
            else
                value &= logicState;
            break;
        default:
            break;
        }


        // FIXME this doesnt work for a OR
        if ((value) && (channel_timeout[sfcfg.specialFuncOutput].On.expired()))
        {
            channel_timeout[sfcfg.specialFuncOutput].Off.stop();
            relays.updateChannel(sfcfg.specialFuncOutput, value);
        }
        else if ((!value) && (channel_timeout[sfcfg.specialFuncOutput].Off.expired()))
        {
            channel_timeout[sfcfg.specialFuncOutput].On.stop();
            relays.updateChannel(sfcfg.specialFuncOutput, value);
        }

        break; // case sftLogic

    case sftBlocking: // blocking function
        startBlocking = (bcu.comObjects->objectRead(COMOBJ_SPECIAL1 + sfcfg.specialFuncNumber) ^ (sfcfg.lockPolarity)) & 0x01;
        endBlocking = false;
        if (startBlocking)
        {
            // action at start of blocking
            blockTyp = sfcfg.blockTypeStart;
        }
        else
        {
            // action at end of blocking
            blockTyp = sfcfg.blockTypeEnd;
            // end blocking, we have to unblock relays
            if (relays.blocked(sfcfg.specialFuncOutput))
            {
                relays.clearBlocked(sfcfg.specialFuncOutput);
                endBlocking = true;
            }
        }

        // change output only on start or end of blocking
        if (startBlocking || endBlocking)
        {
            switch (blockTyp)
            {
            case blNoAction : // no action
                break;
            case blDisable : // disable the output
                relays.updateChannel(sfcfg.specialFuncOutput, false);
                break;
            case blEnable : // enable the output
                relays.updateChannel(sfcfg.specialFuncOutput, true);
                break;
            default:
                break;
            }
        }

        // finally set relays blocked in case of blocking start
        if (startBlocking)
            relays.setBlocked(sfcfg.specialFuncOutput);
        break; // case sftBlocking

    case sftConstrainedLead: // constrained lead
                             // 0x00 no priority, off
                             // 0x01 no priority, on
                             // 0x02 priority, off
                             // 0x03 priority, on
        value = bcu.comObjects->objectRead (COMOBJ_SPECIAL1 + sfcfg.specialFuncNumber);
        if (value & 0b10)
        {   // constrained lead is active for this channel
            // set the value of the special com object as output state
            relays.updateChannel(sfcfg.specialFuncOutput, value & 0b01);
        }
        else
        {   // the constrained lead was just deactivated
            // restore the output based on the object state
            if ((objno >= COMOBJ_SPECIAL1) && ((objno >= COMOBJ_SPECIAL4)))
                relays.updateChannel(sfcfg.specialFuncOutput, bcu.comObjects->objectRead (sfcfg.specialFuncOutput));
        }
        break; // case sftConstrainedLead
    }
}

static unsigned int _handle_timed_functions(const int objno, const unsigned int value)
{
    unsigned int state = tfsUnknown;

    // check that objno is in a valid range
    if ((objno < COMOBJ_INPUT1) || (objno >= int(sizeof(channel_timeout)/sizeof(channel_timeout[0]))))
        return tfsUnknown;

    unsigned int outputState    = relays.channel(objno);
    TimerConfig timercfg = getTimerCfg(objno);

    // channel with no on/off delay or timed function mode
    if (!timercfg.timerOffFactor && !timercfg.timerOnFactor)
    {
        channel_timeout[objno].On.stop();
        channel_timeout[objno].Off.stop();
        return tfsDisabled;
    }

    state = tfsDisabled;
    if (!timercfg.timerMode)
    {   // this is the on/off delay mode
        if (!value && timercfg.timerOffFactor) // Check if a delay is configured for falling edge / Off
        {
            channel_timeout[objno].On.stop();
            channel_timeout[objno].Off.start(timercfg.delayMs * timercfg.timerOffFactor);
            state |= tfsOffDelayed;
        }
        else if (value && timercfg.timerOnFactor) // Check if a delay is configured for raising edge / On
        {
            channel_timeout[objno].Off.stop();
            channel_timeout[objno].On.start(timercfg.delayMs * timercfg.timerOnFactor);
            state |= tfsOnDelayed;
        }
        else
        {
            channel_timeout[objno].On.stop();
            channel_timeout[objno].Off.stop();
            state = tfsDisabled;
        }
    }
    else
    {   // this is the timed function mode (stair way mode)
        if (!timercfg.timerOnFactor) // no delay for on -> switch on the output
        {
            // Check for a timer function without delay factor for raising edge
            if ( !outputState && value)
            {
                relays.setChannel(objno);
                bcu.comObjects->objectWrite(objno, (unsigned int) 1);
                channel_timeout[objno].Off.start (timercfg.delayMs * timercfg.timerOffFactor);
                state |= tfsOffDelayed;
            }
        }
        else
        {
            // Check for a timer function with delay factor for on
            if ( !outputState && value)
            {
                channel_timeout[objno].On.start (timercfg.delayMs * timercfg.timerOnFactor);
                state |= tfsOnDelayed;
            }

            // Check for delay factor for off
            if (outputState && value)
            {   // once the output is ON start the OFF delay
                channel_timeout[objno].Off.start(timercfg.delayMs * timercfg.timerOffFactor);
                state |= tfsOffDelayed;
            }
        }
        // check how to handle off telegram while in timer modus
        if (outputState && !value)
        {   // only switch off if APP_DELAY_ACTION the value is equal zero
            if (!timercfg.timerDelayAction)
            {
                relays.clearChannel(objno);
                bcu.comObjects->objectWrite(objno, (unsigned int) 0);
                channel_timeout[objno].Off.stop();
                state &= ~tfsOffDelayed;
            }
        }
    }
    return state;
}

void objectUpdated(int objno)
{
    unsigned int value = bcu.comObjects->objectRead(objno); // get value of object (0=off, 1=on)

    // check if we have a delayed action for this object, only Outputs
    if (objno < COMOBJ_SPECIAL1) // logic objects must be checked here to
    {
        _handle_timed_functions(objno, value);
        if (channel_timeout[objno].On.stopped() && channel_timeout[objno].Off.stopped())
        {
            relays.updateChannel(objno, value);
            if (value != bcu.comObjects->objectRead(objno)) // objectWrite with set ETS update-flags leads to a infinite loop of objectUpdated & objectWrite
                bcu.comObjects->objectWrite(objno, value);  // see https://selfbus.org/forum/viewtopic.php?p=4762#p4762
        }
    }

    // handle the logic functions for this channel
    _handle_logic_function (objno, value);  //FIXME logic will override on/off delays

    if (relays.pendingChanges())
        _switchObjects(BETWEEN_CHANNEL_DELAY_MS);
}

void checkTimeouts(void)
{
#ifdef HAND_ACTUATION
    // FIXME hand actuation doesn't override logic e.g. blocked channels
    int btnNumber;
    HandActuation::ButtonState btnState;
    if (handAct.getButtonAndState(btnNumber, btnState))
    {
        if (btnState == HandActuation::BUTTON_PRESSED)
        {
            channel_timeout[btnNumber].On.stop();
            channel_timeout[btnNumber].Off.stop();

            if (relays.blocked(btnNumber))
                relays.clearBlocked(btnNumber);

            relays.toggleChannel(btnNumber);
        }
    }
#endif

    // check if we can enable PWM
    relays.checkPWM();
    for (unsigned int objno = 0; objno < (sizeof(channel_timeout)/sizeof(channel_timeout[0])); ++objno)
    {
        unsigned int offTimedout = channel_timeout[objno].Off.expired();
        unsigned int onTimedout = channel_timeout[objno].On.expired();

        if (offTimedout || onTimedout)
        {
            unsigned int newValue = (onTimedout == true);
            relays.updateChannel(objno, newValue);
            bcu.comObjects->objectWrite(objno, newValue);
            _handle_timed_functions(objno, newValue);
            _handle_logic_function(objno, newValue);
        }
    }

    if (relays.pendingChanges())
        _switchObjects(BETWEEN_CHANNEL_DELAY_MS);
}

static void _sendFeedbackObjects(bool forcesendFeedbackObjects)
{
    unsigned int changed = relays.pendingChanges ();

    if (changed || forcesendFeedbackObjects)
    {   // at least one output has changed,
        // -> update the corresponding feedback objects
        unsigned int i;
        unsigned int mask = 0x01;
        unsigned int invert = (*(bcu.userEeprom))[APP_REPORT_BACK_INVERT];
        for (i = 0; i < 8; i++)
        {
            if ((changed & mask) || forcesendFeedbackObjects)
            {   // update feedback object
                unsigned int value = relays.channel(i);
                if (invert & mask)
                    value ^= 0x01;
                bcu.comObjects->objectWrite(COMOBJ_FEEDBACK1 + i, value);
            }
            mask <<= 1;
        }
   }
}

static void _switchObjects(unsigned int delayms)
{
    _sendFeedbackObjects();
    relays.updateOutputs(delayms);
}

void delayAppStart()
{
    const int msMultiplier = 500;
    unsigned int delayAppStartms = 1;
#ifndef DEBUG
    delay(msMultiplier); // always delay a little bit to let the AD-Pin "float"
    // delay the app start by 0.5-10 seconds, so not all out8-apps will return the same time the bus returns.
    analogBegin();  // this needs a re-enable of bus voltage monitoring (in app_main.cpp)
    pinMode(FLOATING_AD_PIN, INPUT_ANALOG);
    for (int i = 0; i<10;i++) // this loop is needed so the analog will be more "random"
    {
        delayAppStartms = analogRead(FLOATING_AD_CHANNEL); // try to read open/floating analog input IO12 /AD5 to get a random number
                                                           // and this needs a re-enable of bus voltage monitoring (in app_main.cpp)
        delay(10);
    }
    pinMode(FLOATING_AD_PIN, INPUT);
    delayAppStartms = (delayAppStartms % 20) * msMultiplier;
#endif

#ifdef BI_STABLE
    if (delayAppStartms < (3*msMultiplier))
        delayAppStartms += 4*msMultiplier; // wait a little bit more, so capacitors can charge and we can switch the relays
#endif

    for (unsigned int i = 0; i<delayAppStartms; i +=msMultiplier)
    {
        delay(msMultiplier);
        int a = !digitalRead(PIN_RUN);
        digitalWrite(PIN_RUN, a); // toggle RUN-LED
    }
#ifdef DEBUG
    digitalWrite(PIN_RUN, 1); // switch RUN-LED ON
#else
    digitalWrite(PIN_RUN, 0); // switch RUN-LED ON
#endif
}

void initApplication(int lastRelayState)
{
    unsigned int i;
    unsigned int initialChannelActions;

    int newRelaystate;
    Outputs::State initialOutputState[NO_OF_CHANNELS];

    relays.setupOutputs(&outputPins[0], NO_OF_OUTPUTS);

    delayAppStart();

    // read & combine initialChannelAction's low & high byte from userEeprom
    // 2 bits for each channel: 0x00=LAST_STATE, 0x01=OPEN, 0x02=CLOSED, e.g. initialChannelAction: 0xAAAA Ch1-8 closed; 0x5555 Ch1-8 open; 0x0000 Ch1-8 last state
    initialChannelActions = ((*(bcu.userEeprom))[APP_RESTORE_AFTER_PL_HI] << 8) | (*(bcu.userEeprom))[APP_RESTORE_AFTER_PL_LO];

    newRelaystate = 0x00;
    for (i=0; i < (sizeof(initialOutputState)/sizeof(initialOutputState[0])); i++)
    {
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
        // create the relays begin state according to the requested initial output states
        if (initialOutputState[i] == Outputs::CLOSED)
            newRelaystate |= 1 << i;
    }

    // set all logic objects to false
    for (i=COMOBJ_SPECIAL1; i <= COMOBJ_SPECIAL4; i++)
        bcu.comObjects->objectSetValue(i, (unsigned int) 0);

    // set all output objects according to configured initial output state
    for (i=COMOBJ_INPUT1; i < (sizeof(initialOutputState)/sizeof(initialOutputState[0])); i++)
    {
        unsigned int value = (initialOutputState[i] == Outputs::CLOSED);
        bcu.comObjects->objectSetValue(i, value);
    }

    /*
     * according to the jung manual, outputs will be switched on off, ignoring timed functions or logics
     *
    // only set output , if its not delayed on or in a timed configuration, or the logic prohibits it
    for (i=COMOBJ_INPUT1; i < (sizeof(initialOutputState)/sizeof(initialOutputState[0])); i++)
    {
        unsigned int delayed = _init_timed_functions(i, objectRead(i));
        if ((delayed & tfsOnDelayed) == tfsOnDelayed)  // check for timed functions for the channel
            newRelaystate &= ~(1 << i);

        logicResult logicRes = _init_logic_function(i, objectRead(i)); // initialize the logic functions for the channel
        switch (logicRes)
        {
            case lrSetOn:
                newRelaystate |= (1 << i);
                break;
            case lrSetOff:
                newRelaystate &= ~(1 << i);
                break;
            default:
                break;
        }
    }
    */
    // check logic functions, maybe channels need to be blocked
    for (i=COMOBJ_INPUT1; i < (sizeof(initialOutputState)/sizeof(initialOutputState[0])); i++)
        _handle_logic_function(i, bcu.comObjects->objectRead(i)); // handle the logic functions for the channel

    // set the initial relays state, this needs to be done as last operation before real
    relays.begin(newRelaystate, (*(bcu.userEeprom))[APP_CLOSER_MODE], NO_OF_CHANNELS);

#ifdef HAND_ACTUATION
    handAct.setDelayBetweenButtonsMs(10);
    handAct.setDelayAtEndMs(10);
    relays.setHandActuation(&handAct);
#endif

    // switch the relays according to newRelaystate and send feedback objects

    _switchObjects();
}

void stopApplication()
{
    // stop all running timers
    for (unsigned int i = 0; i < (sizeof(channel_timeout)/sizeof(channel_timeout[0])); i++)
    {
      channel_timeout[i].Off.stop();
      channel_timeout[i].On.stop();
    }

#ifndef BI_STABLE
    pinMode(PIN_PWM, OUTPUT); //switch off PWM for mono-stable relays
    digitalWrite(PIN_PWM, 1);
#endif

    //switch off all possible active relay coils, to save some power
    for (unsigned int i = 0; i < sizeof(outputPins)/sizeof(outputPins[0]); i++)
        digitalWrite(outputPins[0], 0);


#ifdef HAND_ACTUATION
    // switch all hand actuation LEDs off, to save some power
    handAct.setallLedState(false);
#endif

    // finally stop the bcu
    bcu.end();
}

unsigned int getRelaysState()
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
/*
static unsigned int _init_timed_functions(const int objno, const unsigned int value)
{
    unsigned int state = tfsUnknown;

    // check that objno is in a valid range
    if ((objno < COMOBJ_INPUT1) || (objno >= int(sizeof(channel_timeout)/sizeof(channel_timeout[0]))))
        return tfsUnknown;

    TimerConfig timercfg = getTimerCfg(objno);

    // channel with no on/off delay or timed function mode
    if (!timercfg.timerOffFactor && !timercfg.timerOnFactor)
    {
        channel_timeout[objno].On.stop();
        channel_timeout[objno].Off.stop();
        return tfsDisabled;
    }

    state = tfsDisabled;
    if (!timercfg.timerMode)
    {   // this is the on/off delay mode
        if (!value && timercfg.timerOffFactor) // Check if a delay is configured for falling edge / Off
        {
            channel_timeout[objno].On.stop();
            channel_timeout[objno].Off.start(timercfg.delayMs * timercfg.timerOffFactor);
            state |= tfsOffDelayed;
        }
        else if (value && timercfg.timerOnFactor)  // Check if a delay is configured for raising edge / On
        {
            channel_timeout[objno].Off.stop();
            channel_timeout[objno].On.start(timercfg.delayMs * timercfg.timerOnFactor);
            state |= tfsOnDelayed;
        }
        else
        {
            channel_timeout[objno].On.stop();
            channel_timeout[objno].Off.stop();
            state = tfsDisabled;
        }
    }
    else
    {   // this is the timed function mode (stair way mode)
        if (value)
        {
            if (timercfg.timerOffFactor ) // channel with off delay
            {
                channel_timeout[objno].Off.start(timercfg.delayMs * timercfg.timerOffFactor);
                state |= tfsOffDelayed;
            }

            if (timercfg.timerOnFactor) // channel with on delay
            {
                channel_timeout[objno].On.start(timercfg.delayMs * timercfg.timerOnFactor);
                state |= tfsOnDelayed;
            }
        }
    }
    return state;
}

logicResult _init_logic_function(const int objno, unsigned int value)
{
    SpecialFunctionConfig sfcfg = getSpecialFunctionConfig(objno);

    bool startBlocking;                 // true if a blocking is started
    unsigned int logicState;            // state of logic function
    unsigned int valueConstrainedLead;  //

    logicResult logicRes = lrUnchanged;

    // output objects are from 0->7 & special functions are from 8->11, otherwise return
    if ((objno < COMOBJ_INPUT1) || (objno > COMOBJ_SPECIAL4))
        return lrUnchanged;

    // special functions are from 8->11, need the value from the connected output
    if ((objno >= COMOBJ_SPECIAL1) && (objno <= COMOBJ_SPECIAL4))
    {
        if ((sfcfg.specialFuncOutput > COMOBJ_INPUT1) && (sfcfg.specialFuncOutput < (COMOBJ_INPUT1+NO_OF_CHANNELS)))
            value = objectRead(sfcfg.specialFuncOutput);
    }

    switch (sfcfg.Mode)
    {
    case sftUnknown :
         break; // sftUnknown

    case sftLogic : // logic function (OR/AND/AND with recirculation
        // get the logic state for the special function object
        logicState = objectRead(COMOBJ_SPECIAL1 + sfcfg.specialFuncNumber);
        switch (sfcfg.logicFuncTyp)
        {
        case ltOR : // or
            value |= logicState;
            break;
        case ltAND : // and
            value &= logicState;
            break;
        case ltAND_RECIRC : // and with recirculation
            // UND mit Rückführung:
            // Verknüpfungs-Objekt = "0" Ausgang ist immer "0" (logisch UND).
            // In diesem Fall wird durch die Rückführung des Ausgangs auf das Schalten-Objekt dieses beim Setzen wieder zurückgesetzt.
            // Erst, wenn das Verknüpfungs-Objekt = "1" ist, kann durch eine neu empfangene "1" am Schalten-Objekt der
            // Ausgang den logischen Zustand "1" annehmen.
            if (!logicState)
            {
                // if the logic part of the and connection has been
                // cleared -> clear also the real object value
                objectSetValue(sfcfg.specialFuncOutput, false);
                value = false;
            }
            else
                value &= logicState;
            break;
        default:
            break;
        }

        if (sfcfg.logicFuncTyp != ltUnknown)
        {
            if (value)
               logicRes = lrSetOn;
            else
               logicRes = lrSetOff;
        }
        break; // case sftLogic

    case sftBlocking: // blocking function
        startBlocking = (objectRead(COMOBJ_SPECIAL1 + sfcfg.specialFuncNumber) ^ (sfcfg.lockPolarity)) & 0x01;
        // change output only on start or end of blocking
        if (startBlocking)
        {
            switch (sfcfg.blockTypeStart)
            {
            case blNoAction : // no action
                break;
            case blDisable : // disable the output
                value = false;
                logicRes = lrSetOff;
                break;
            case blEnable : // enable the output
                value = true;
                logicRes = lrSetOn;
                break;
            default:
                break;
            }
        }
        break; // case sftBlocking

    case sftConstrainedLead: // constrained lead
                             // 0x00 no priority, off
                             // 0x01 no priority, on
                             // 0x02 priority, off
                             // 0x03 priority, on
        valueConstrainedLead = objectRead (COMOBJ_SPECIAL1 + sfcfg.specialFuncNumber);
        if (valueConstrainedLead & 0b10)
        {   // constrained lead is active for this channel
            if (valueConstrainedLead & 0b01)
                logicRes = lrSetOn;
            else
                logicRes = lrSetOff;
        }
        break; // case sftConstrainedLead
    default:
        break; //default
    }
    return logicRes;
}
*/
