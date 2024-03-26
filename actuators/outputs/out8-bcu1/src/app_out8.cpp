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

enum specialFunctionType {sftUnknown = -1, sftLogic = 0, sftBlocking = 1, sftForcedPositioning = 2}; // Verknuepfungsobjekt, Sperrobjekt, Zwangsstellungsobjekt
enum logicType {ltUnknown = 0, ltOR = 1, ltAND = 2, ltAND_RECIRC = 3}; // logic OR, AND, AND with recirculation (UND mit Rueckfuehrung)
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
static void          _handle_logic_function(int16_t objno, uint8_t value);
static unsigned int  _handle_timed_functions(const int objno, const unsigned int value);


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
        case sftForcedPositioning: // Zwangsstellung
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

void handleBooleanLogic(const SpecialFunctionConfig cfg)
{
    bool logicState = bcu.comObjects->objectRead(COMOBJ_SPECIAL1 + cfg.specialFuncNumber); // state of logic function
    uint8_t affectedOutput = cfg.specialFuncOutput;
    bool value = bcu.comObjects->objectRead(affectedOutput); // read the value of the affected output 0-7

    switch (cfg.logicFuncTyp)
    {
        case ltOR:
            value |= logicState;
            break;
        case ltAND: // and
            value &= logicState;
            break;
        case ltAND_RECIRC: // AND with recirculation
             // UND mit Rückführung:
             // Verknüpfungs-Objekt = "0" Ausgang ist immer "0" (logisch UND).
             // In diesem Fall wird durch die Rückführung des Ausgangs auf das Schalten-Objekt dieses beim Setzen wieder zurückgesetzt.
             // Erst, wenn das Verknüpfungs-Objekt = "1" ist, kann durch eine neu empfangene "1" am Schalten-Objekt der
             // Ausgang den logischen Zustand "1" annehmen.
            if (!logicState)
            {
                // if the logic part of the AND connection has been
                // cleared -> clear also the affected output state
                bcu.comObjects->objectSetValue(affectedOutput, false);
                value = false;
                channel_timeout[affectedOutput].On.stop();
                channel_timeout[affectedOutput].Off.stop();
            }
            else
                value &= logicState;
            break;
        default:
            break;
    }

    if (value)
    {
        channel_timeout[cfg.specialFuncOutput].Off.stop();
    }
    else
    {
        channel_timeout[cfg.specialFuncOutput].On.stop();
    }
    relays.updateChannel(cfg.specialFuncOutput, value);
}

void handleBlockingLogic(const SpecialFunctionConfig cfg)
{
    uint8_t affectedOutput = cfg.specialFuncOutput;
    bool startBlocking; // true if a blocking is started
    bool endBlocking;   // true if a blocking was ended
    blockType blockTyp; // holds the blocking type

    startBlocking = (bcu.comObjects->objectRead(COMOBJ_SPECIAL1 + cfg.specialFuncNumber) ^ (cfg.lockPolarity)) & 0x01;
    endBlocking = false;
    if (startBlocking)
    {
        // action at start of blocking
        blockTyp = cfg.blockTypeStart;
    }
    else
    {
        // action at end of blocking
        blockTyp = cfg.blockTypeEnd;
        // end blocking, we have to unblock relays
        if (relays.blocked(affectedOutput))
        {
            relays.clearBlocked(affectedOutput);
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
            relays.updateChannel(affectedOutput, false);
            break;
        case blEnable : // enable the output
            relays.updateChannel(affectedOutput, true);
            break;
        default:
            break;
        }
    }

    // finally set relays blocked in case of blocking start
    if (startBlocking)
    {
        relays.setBlocked(affectedOutput);
    }
}

void handleForcedPositioning(const SpecialFunctionConfig cfg, const int16_t objno)
{
    // 0b00 no priority, off
    // 0b01 no priority, on
    // 0b10    priority, off
    // 0b11    priority, on
    uint8_t value = bcu.comObjects->objectRead (COMOBJ_SPECIAL1 + cfg.specialFuncNumber);
    if (value & 0b10)
    {   // priority is active for this channel
        // set the value of the special com object as output state
        relays.updateChannel(cfg.specialFuncOutput, value & 0b01);
    }
    else
    {   // priority was just deactivated
        // restore the output based on the object state
        if ((objno >= COMOBJ_SPECIAL1) && ((objno <= COMOBJ_SPECIAL4)))
            relays.updateChannel(cfg.specialFuncOutput, bcu.comObjects->objectRead(cfg.specialFuncOutput));
    }
}

static void _handle_logic_function(int16_t objno, uint8_t value)
{
    SpecialFunctionConfig config = getSpecialFunctionConfig(objno);

    ///\todo Logic handling is mostly untested
    switch (config.Mode)
    {
        case sftLogic: // OR/AND/AND with recirculation
            handleBooleanLogic(config);
            break;

        case sftBlocking:
            handleBlockingLogic(config);
            break;

        case sftForcedPositioning: // Zwangsstellung
                handleForcedPositioning(config, objno);
            break;

        case sftUnknown:
            break;

        default:
            break;
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
    unsigned int value = bcu.comObjects->objectRead(objno); // 0=off, 1=on or 2 bits for forced positioning (Zwangsstellung)

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
    _handle_logic_function (objno, value);  ///\todo boolean logic will override on/off delays

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
            bcu.comObjects->objectWrite(COMOBJ_FEEDBACK1 + objno, newValue);
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

    // according to the jung manual, outputs are switched on/off on startup, ignoring timed functions or logics
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
