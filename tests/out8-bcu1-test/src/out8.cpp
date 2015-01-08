/*
 *  out8.cpp - Basic tests for the simple case
 *
 *  Copyright (c) 2014 Martin GlÃ¼ck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "protocol.h"
#include "app_out8.h"
#include "catch.hpp"
#include "sblib/timer.h"

typedef struct
{
    unsigned int outputs;
    unsigned int pwm_on;
} Out8TestState;

static Out8TestState _refState;
static Out8TestState _stepState;

static void _gatherState (Out8TestState * state, Out8TestState * refState)
{
    int i;
    state->outputs = 0;
    state->pwm_on  = timer16_0.match(PWM0) != 0;
    for (i=0; i < 8; i++)
    {
        state->outputs |= (digitalRead(outputPins[i]) << i);
    }
    if (refState)
    {
        REQUIRE(state->outputs == refState->outputs);
        REQUIRE(state->pwm_on  == refState->pwm_on);
    }
}

static void _loop(Out8TestState * refState)
{
    bcu.loop();
    loop();
}

static void _output1Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x01;
    refState->pwm_on   = 0;
}

static void _output2Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x02;
    refState->pwm_on   = 0;
}

static void _output3Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x04;
    refState->pwm_on   = 0;
}

static void _output4Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x08;
    refState->pwm_on   = 0;
}

static void _output12Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x03;
    refState->pwm_on   = 0;
}

static void _enablePWM(Out8TestState * refState)
{
    _loop(refState);
    refState->pwm_on = 0x01;
}

static void _output1Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x01;
}
static void _output2Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x02;
}
static void _output3Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x04;
}
static void _output4Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x08;
}

#ifdef CLEAR12
static void _output12Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x03;
}
#endif

#if 1
// >>> TC:simple
// Date: 2014-06-27 13:13:34.767017

/* Code for test case simple */
static void simple_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-06-27 13:13:34.767017
    // Assoc Table (0x15D):
    //    1 ( 1/0/20) <-> 12 (feedback 1          ) @ 0x15E
    //    2 ( 1/0/30) <->  0 (output 1            ) @ 0x160
    //    2 ( 1/0/30) <->  1 (output 2            ) @ 0x162
    //    3 ( 1/0/31) <->  1 (output 2            ) @ 0x164
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/20 @ 0x119
    //   ( 2)  1/0/30 @ 0x11B
    //   ( 3)  1/0/31 @ 0x11D
    // Com Object table (0x11F):
    //   ( 0) output 1             <6B, 17, 00> @ 0x121
    //   ( 1) output 2             <6C, 17, 00> @ 0x124
    //   ( 2) output 3             <6D, 17, 00> @ 0x127
    //   ( 3) output 4             <6E, 17, 00> @ 0x12A
    //   ( 4) output 5             <6F, 17, 00> @ 0x12D
    //   ( 5) output 6             <70, 17, 00> @ 0x130
    //   ( 6) output 7             <71, 17, 00> @ 0x133
    //   ( 7) output 8             <72, 17, 00> @ 0x136
    //   ( 8) special 1            <73, 1F, 01> @ 0x139
    //   ( 9) special 2            <74, 1F, 01> @ 0x13C
    //   (10) special 3            <75, 1F, 01> @ 0x13F
    //   (11) special 4            <76, 1F, 01> @ 0x142
    //   (12) feedback 1           <77, 4F, 00> @ 0x145
    //   (13) feedback 2           <78, 4F, 00> @ 0x148
    //   (14) feedback 3           <79, 4F, 00> @ 0x14B
    //   (15) feedback 4           <7A, 4F, 00> @ 0x14E
    //   (16) feedback 5           <7B, 4F, 00> @ 0x151
    //   (17) feedback 6           <7C, 4F, 00> @ 0x154
    //   (18) feedback 7           <7D, 4F, 00> @ 0x157
    //   (19) feedback 8           <7E, 4F, 00> @ 0x15A
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x60;
    userEeprom[0x106] = 0x20;
    userEeprom[0x107] = 0x01;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x5D;
    userEeprom[0x112] = 0x1F;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x03;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x14;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x1E;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x1F;
    userEeprom[0x11F] = 0x14;
    userEeprom[0x120] = 0x61;
    userEeprom[0x121] = 0x6B;
    userEeprom[0x122] = 0x17;
    userEeprom[0x123] = 0x00;
    userEeprom[0x124] = 0x6C;
    userEeprom[0x125] = 0x17;
    userEeprom[0x126] = 0x00;
    userEeprom[0x127] = 0x6D;
    userEeprom[0x128] = 0x17;
    userEeprom[0x129] = 0x00;
    userEeprom[0x12A] = 0x6E;
    userEeprom[0x12B] = 0x17;
    userEeprom[0x12C] = 0x00;
    userEeprom[0x12D] = 0x6F;
    userEeprom[0x12E] = 0x17;
    userEeprom[0x12F] = 0x00;
    userEeprom[0x130] = 0x70;
    userEeprom[0x131] = 0x17;
    userEeprom[0x132] = 0x00;
    userEeprom[0x133] = 0x71;
    userEeprom[0x134] = 0x17;
    userEeprom[0x135] = 0x00;
    userEeprom[0x136] = 0x72;
    userEeprom[0x137] = 0x17;
    userEeprom[0x138] = 0x00;
    userEeprom[0x139] = 0x73;
    userEeprom[0x13A] = 0x1F;
    userEeprom[0x13B] = 0x01;
    userEeprom[0x13C] = 0x74;
    userEeprom[0x13D] = 0x1F;
    userEeprom[0x13E] = 0x01;
    userEeprom[0x13F] = 0x75;
    userEeprom[0x140] = 0x1F;
    userEeprom[0x141] = 0x01;
    userEeprom[0x142] = 0x76;
    userEeprom[0x143] = 0x1F;
    userEeprom[0x144] = 0x01;
    userEeprom[0x145] = 0x77;
    userEeprom[0x146] = 0x4F;
    userEeprom[0x147] = 0x00;
    userEeprom[0x148] = 0x78;
    userEeprom[0x149] = 0x4F;
    userEeprom[0x14A] = 0x00;
    userEeprom[0x14B] = 0x79;
    userEeprom[0x14C] = 0x4F;
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x7A;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x7B;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x7C;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x7D;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
    userEeprom[0x15A] = 0x7E;
    userEeprom[0x15B] = 0x4F;
    userEeprom[0x15C] = 0x00;
    userEeprom[0x15D] = 0x04;
    userEeprom[0x15E] = 0x01;
    userEeprom[0x15F] = 0x0C;
    userEeprom[0x160] = 0x02;
    userEeprom[0x161] = 0x00;
    userEeprom[0x162] = 0x02;
    userEeprom[0x163] = 0x01;
    userEeprom[0x164] = 0x03;
    userEeprom[0x165] = 0x01;
    userEeprom[0x166] = 0x00;
    userEeprom[0x167] = 0x00;
    userEeprom[0x168] = 0x00;
    userEeprom[0x169] = 0x00;
    userEeprom[0x16A] = 0x00;
    userEeprom[0x16B] = 0x00;
    userEeprom[0x16C] = 0x00;
    userEeprom[0x16D] = 0x00;
    userEeprom[0x16E] = 0x00;
    userEeprom[0x16F] = 0x00;
    userEeprom[0x170] = 0x00;
    userEeprom[0x171] = 0x00;
    userEeprom[0x172] = 0x00;
    userEeprom[0x173] = 0x00;
    userEeprom[0x174] = 0x00;
    userEeprom[0x175] = 0x00;
    userEeprom[0x176] = 0x00;
    userEeprom[0x177] = 0x00;
    userEeprom[0x178] = 0x00;
    userEeprom[0x179] = 0x00;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x00;
    userEeprom[0x17C] = 0x00;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x00;
    userEeprom[0x17F] = 0x00;
    userEeprom[0x180] = 0x00;
    userEeprom[0x181] = 0x00;
    userEeprom[0x182] = 0x00;
    userEeprom[0x183] = 0x00;
    userEeprom[0x184] = 0x00;
    userEeprom[0x185] = 0x00;
    userEeprom[0x186] = 0x00;
    userEeprom[0x187] = 0x00;
    userEeprom[0x188] = 0x00;
    userEeprom[0x189] = 0x00;
    userEeprom[0x18A] = 0x00;
    userEeprom[0x18B] = 0x00;
    userEeprom[0x18C] = 0x00;
    userEeprom[0x18D] = 0x00;
    userEeprom[0x18E] = 0x00;
    userEeprom[0x18F] = 0x00;
    userEeprom[0x190] = 0x00;
    userEeprom[0x191] = 0x00;
    userEeprom[0x192] = 0x00;
    userEeprom[0x193] = 0x00;
    userEeprom[0x194] = 0x00;
    userEeprom[0x195] = 0x00;
    userEeprom[0x196] = 0x00;
    userEeprom[0x197] = 0x00;
    userEeprom[0x198] = 0x00;
    userEeprom[0x199] = 0x00;
    userEeprom[0x19A] = 0x00;
    userEeprom[0x19B] = 0x00;
    userEeprom[0x19C] = 0x00;
    userEeprom[0x19D] = 0x00;
    userEeprom[0x19E] = 0x00;
    userEeprom[0x19F] = 0x00;
    userEeprom[0x1A0] = 0x00;
    userEeprom[0x1A1] = 0x00;
    userEeprom[0x1A2] = 0x00;
    userEeprom[0x1A3] = 0x00;
    userEeprom[0x1A4] = 0x00;
    userEeprom[0x1A5] = 0x00;
    userEeprom[0x1A6] = 0x00;
    userEeprom[0x1A7] = 0x00;
    userEeprom[0x1A8] = 0x00;
    userEeprom[0x1A9] = 0x00;
    userEeprom[0x1AA] = 0x00;
    userEeprom[0x1AB] = 0x00;
    userEeprom[0x1AC] = 0x00;
    userEeprom[0x1AD] = 0x00;
    userEeprom[0x1AE] = 0x00;
    userEeprom[0x1AF] = 0x00;
    userEeprom[0x1B0] = 0x00;
    userEeprom[0x1B1] = 0x00;
    userEeprom[0x1B2] = 0x00;
    userEeprom[0x1B3] = 0x00;
    userEeprom[0x1B4] = 0x00;
    userEeprom[0x1B5] = 0x00;
    userEeprom[0x1B6] = 0x00;
    userEeprom[0x1B7] = 0x00;
    userEeprom[0x1B8] = 0x00;
    userEeprom[0x1B9] = 0x00;
    userEeprom[0x1BA] = 0x00;
    userEeprom[0x1BB] = 0x00;
    userEeprom[0x1BC] = 0x00;
    userEeprom[0x1BD] = 0x00;
    userEeprom[0x1BE] = 0x00;
    userEeprom[0x1BF] = 0x00;
    userEeprom[0x1C0] = 0x00;
    userEeprom[0x1C1] = 0x00;
    userEeprom[0x1C2] = 0x00;
    userEeprom[0x1C3] = 0x00;
    userEeprom[0x1C4] = 0x00;
    userEeprom[0x1C5] = 0x00;
    userEeprom[0x1C6] = 0x00;
    userEeprom[0x1C7] = 0x00;
    userEeprom[0x1C8] = 0x00;
    userEeprom[0x1C9] = 0x00;
    userEeprom[0x1CA] = 0x00;
    userEeprom[0x1CB] = 0x00;
    userEeprom[0x1CC] = 0x00;
    userEeprom[0x1CD] = 0x00;
    userEeprom[0x1CE] = 0x00;
    userEeprom[0x1CF] = 0x00;
    userEeprom[0x1D0] = 0x00;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x00;
    userEeprom[0x1D3] = 0x00;
    userEeprom[0x1D4] = 0x00;
    userEeprom[0x1D5] = 0x00;
    userEeprom[0x1D6] = 0x00;
    userEeprom[0x1D7] = 0x00;
    userEeprom[0x1D8] = 0x00;
    userEeprom[0x1D9] = 0x00;
    userEeprom[0x1DA] = 0x00;
    userEeprom[0x1DB] = 0x00;
    userEeprom[0x1DC] = 0x00;
    userEeprom[0x1DD] = 0x00;
    userEeprom[0x1DE] = 0x00;
    userEeprom[0x1DF] = 0x00;
    userEeprom[0x1E0] = 0x00;
    userEeprom[0x1E1] = 0x00;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x00;
    userEeprom[0x1E4] = 0x00;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x00;
    userEeprom[0x1E8] = 0x00;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x00;
    userEeprom[0x1EB] = 0x00;
    userEeprom[0x1EC] = 0x00;
    userEeprom[0x1ED] = 0x00;
    userEeprom[0x1EE] = 0x00;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x00;
    userEeprom[0x1F2] = 0x00;
    userEeprom[0x1F3] = 0x00;
    userEeprom[0x1F4] = 0x00;
    userEeprom[0x1F5] = 0x00;
    userEeprom[0x1F6] = 0x00;
    userEeprom[0x1F7] = 0x00;
    userEeprom[0x1F8] = 0x00;
    userEeprom[0x1F9] = 0x00;
    userEeprom[0x1FA] = 0x00;
    userEeprom[0x1FB] = 0x00;
    userEeprom[0x1FC] = 0x00;
    userEeprom[0x1FD] = 0x00;
    userEeprom[0x1FE] = 0x00;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_simple[] =
{
  {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   1
  // check after the init of the application that the feedback object will be sent
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x80}} //   2
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   3
  // only for one feedback channel a group address is store -> only one telegram should be sent
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //   4

  // receive a "ON" telegram for output 1 and output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}} //   5
  // process the received telegram inside the app
  // since there is a switch from OFF to ON, the PWM timer should be started
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output12Set        , {}} //   6
  // another app loop is required for the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   7
  // as a reaction to the change a feedback telegram should be sent
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x81}} //   8
  // only for one feedback channel a group address is store -> only one telegram should be sent
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //   9
, {TIMER_TICK     , 10,  0, (StepFunction *) _enablePWM          , {}} //  10

  // receive a "OFF" telegram for output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}} //  11
  // process the received telegram inside the app
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Clear       , {}} //  12
, {END}
};
static Test_Case simple_tc =
{
  "OUT8 - Test 1"
, 0x0004, 0x2060, 01
, 0 // power-on delay
, simple_eepromSetup
, NULL
, (StateFunction *) _gatherState
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_simple
};

TEST_CASE("OUT8 - Test 1","[SIMPLE]")
{
  executeTest(& simple_tc);
}
// <<< TC:simple
#endif

#if 1
// >>> TC:simple_timeout
// Date: 2014-06-27 13:13:34.822020

/* Code for test case simple_timeout */
static void simple_timeout_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-06-27 13:13:34.822020
    // Assoc Table (0x15F):
    //    1 ( 1/0/20) <-> 12 (feedback 1          ) @ 0x160
    //    2 ( 1/0/21) <-> 13 (feedback 2          ) @ 0x162
    //    3 ( 1/0/30) <->  0 (output 1            ) @ 0x164
    //    4 ( 1/0/31) <->  1 (output 2            ) @ 0x166
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/20 @ 0x119
    //   ( 2)  1/0/21 @ 0x11B
    //   ( 3)  1/0/30 @ 0x11D
    //   ( 4)  1/0/31 @ 0x11F
    // Com Object table (0x121):
    //   ( 0) output 1             <6B, 17, 00> @ 0x123
    //   ( 1) output 2             <6C, 17, 00> @ 0x126
    //   ( 2) output 3             <6D, 17, 00> @ 0x129
    //   ( 3) output 4             <6E, 17, 00> @ 0x12C
    //   ( 4) output 5             <6F, 17, 00> @ 0x12F
    //   ( 5) output 6             <70, 17, 00> @ 0x132
    //   ( 6) output 7             <71, 17, 00> @ 0x135
    //   ( 7) output 8             <72, 17, 00> @ 0x138
    //   ( 8) special 1            <73, 1F, 01> @ 0x13B
    //   ( 9) special 2            <74, 1F, 01> @ 0x13E
    //   (10) special 3            <75, 1F, 01> @ 0x141
    //   (11) special 4            <76, 1F, 01> @ 0x144
    //   (12) feedback 1           <77, 4F, 00> @ 0x147
    //   (13) feedback 2           <78, 4F, 00> @ 0x14A
    //   (14) feedback 3           <79, 4F, 00> @ 0x14D
    //   (15) feedback 4           <7A, 4F, 00> @ 0x150
    //   (16) feedback 5           <7B, 4F, 00> @ 0x153
    //   (17) feedback 6           <7C, 4F, 00> @ 0x156
    //   (18) feedback 7           <7D, 4F, 00> @ 0x159
    //   (19) feedback 8           <7E, 4F, 00> @ 0x15C
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x60;
    userEeprom[0x106] = 0x20;
    userEeprom[0x107] = 0x01;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x5F;
    userEeprom[0x112] = 0x21;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x04;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x14;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x15;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x1E;
    userEeprom[0x11F] = 0x08;
    userEeprom[0x120] = 0x1F;
    userEeprom[0x121] = 0x14;
    userEeprom[0x122] = 0x61;
    userEeprom[0x123] = 0x6B;
    userEeprom[0x124] = 0x17;
    userEeprom[0x125] = 0x00;
    userEeprom[0x126] = 0x6C;
    userEeprom[0x127] = 0x17;
    userEeprom[0x128] = 0x00;
    userEeprom[0x129] = 0x6D;
    userEeprom[0x12A] = 0x17;
    userEeprom[0x12B] = 0x00;
    userEeprom[0x12C] = 0x6E;
    userEeprom[0x12D] = 0x17;
    userEeprom[0x12E] = 0x00;
    userEeprom[0x12F] = 0x6F;
    userEeprom[0x130] = 0x17;
    userEeprom[0x131] = 0x00;
    userEeprom[0x132] = 0x70;
    userEeprom[0x133] = 0x17;
    userEeprom[0x134] = 0x00;
    userEeprom[0x135] = 0x71;
    userEeprom[0x136] = 0x17;
    userEeprom[0x137] = 0x00;
    userEeprom[0x138] = 0x72;
    userEeprom[0x139] = 0x17;
    userEeprom[0x13A] = 0x00;
    userEeprom[0x13B] = 0x73;
    userEeprom[0x13C] = 0x1F;
    userEeprom[0x13D] = 0x01;
    userEeprom[0x13E] = 0x74;
    userEeprom[0x13F] = 0x1F;
    userEeprom[0x140] = 0x01;
    userEeprom[0x141] = 0x75;
    userEeprom[0x142] = 0x1F;
    userEeprom[0x143] = 0x01;
    userEeprom[0x144] = 0x76;
    userEeprom[0x145] = 0x1F;
    userEeprom[0x146] = 0x01;
    userEeprom[0x147] = 0x77;
    userEeprom[0x148] = 0x4F;
    userEeprom[0x149] = 0x00;
    userEeprom[0x14A] = 0x78;
    userEeprom[0x14B] = 0x4F;
    userEeprom[0x14C] = 0x00;
    userEeprom[0x14D] = 0x79;
    userEeprom[0x14E] = 0x4F;
    userEeprom[0x14F] = 0x00;
    userEeprom[0x150] = 0x7A;
    userEeprom[0x151] = 0x4F;
    userEeprom[0x152] = 0x00;
    userEeprom[0x153] = 0x7B;
    userEeprom[0x154] = 0x4F;
    userEeprom[0x155] = 0x00;
    userEeprom[0x156] = 0x7C;
    userEeprom[0x157] = 0x4F;
    userEeprom[0x158] = 0x00;
    userEeprom[0x159] = 0x7D;
    userEeprom[0x15A] = 0x4F;
    userEeprom[0x15B] = 0x00;
    userEeprom[0x15C] = 0x7E;
    userEeprom[0x15D] = 0x4F;
    userEeprom[0x15E] = 0x00;
    userEeprom[0x15F] = 0x04;
    userEeprom[0x160] = 0x01;
    userEeprom[0x161] = 0x0C;
    userEeprom[0x162] = 0x02;
    userEeprom[0x163] = 0x0D;
    userEeprom[0x164] = 0x03;
    userEeprom[0x165] = 0x00;
    userEeprom[0x166] = 0x04;
    userEeprom[0x167] = 0x01;
    userEeprom[0x168] = 0x00;
    userEeprom[0x169] = 0x00;
    userEeprom[0x16A] = 0x00;
    userEeprom[0x16B] = 0x00;
    userEeprom[0x16C] = 0x00;
    userEeprom[0x16D] = 0x00;
    userEeprom[0x16E] = 0x00;
    userEeprom[0x16F] = 0x00;
    userEeprom[0x170] = 0x00;
    userEeprom[0x171] = 0x00;
    userEeprom[0x172] = 0x00;
    userEeprom[0x173] = 0x00;
    userEeprom[0x174] = 0x00;
    userEeprom[0x175] = 0x00;
    userEeprom[0x176] = 0x00;
    userEeprom[0x177] = 0x00;
    userEeprom[0x178] = 0x00;
    userEeprom[0x179] = 0x00;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x00;
    userEeprom[0x17C] = 0x00;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x00;
    userEeprom[0x17F] = 0x00;
    userEeprom[0x180] = 0x00;
    userEeprom[0x181] = 0x00;
    userEeprom[0x182] = 0x00;
    userEeprom[0x183] = 0x00;
    userEeprom[0x184] = 0x00;
    userEeprom[0x185] = 0x00;
    userEeprom[0x186] = 0x00;
    userEeprom[0x187] = 0x00;
    userEeprom[0x188] = 0x00;
    userEeprom[0x189] = 0x00;
    userEeprom[0x18A] = 0x00;
    userEeprom[0x18B] = 0x00;
    userEeprom[0x18C] = 0x00;
    userEeprom[0x18D] = 0x00;
    userEeprom[0x18E] = 0x00;
    userEeprom[0x18F] = 0x00;
    userEeprom[0x190] = 0x00;
    userEeprom[0x191] = 0x00;
    userEeprom[0x192] = 0x00;
    userEeprom[0x193] = 0x00;
    userEeprom[0x194] = 0x00;
    userEeprom[0x195] = 0x00;
    userEeprom[0x196] = 0x00;
    userEeprom[0x197] = 0x00;
    userEeprom[0x198] = 0x00;
    userEeprom[0x199] = 0x00;
    userEeprom[0x19A] = 0x00;
    userEeprom[0x19B] = 0x00;
    userEeprom[0x19C] = 0x00;
    userEeprom[0x19D] = 0x00;
    userEeprom[0x19E] = 0x00;
    userEeprom[0x19F] = 0x00;
    userEeprom[0x1A0] = 0x00;
    userEeprom[0x1A1] = 0x00;
    userEeprom[0x1A2] = 0x00;
    userEeprom[0x1A3] = 0x00;
    userEeprom[0x1A4] = 0x00;
    userEeprom[0x1A5] = 0x00;
    userEeprom[0x1A6] = 0x00;
    userEeprom[0x1A7] = 0x00;
    userEeprom[0x1A8] = 0x00;
    userEeprom[0x1A9] = 0x00;
    userEeprom[0x1AA] = 0x00;
    userEeprom[0x1AB] = 0x00;
    userEeprom[0x1AC] = 0x00;
    userEeprom[0x1AD] = 0x00;
    userEeprom[0x1AE] = 0x00;
    userEeprom[0x1AF] = 0x00;
    userEeprom[0x1B0] = 0x00;
    userEeprom[0x1B1] = 0x00;
    userEeprom[0x1B2] = 0x00;
    userEeprom[0x1B3] = 0x00;
    userEeprom[0x1B4] = 0x00;
    userEeprom[0x1B5] = 0x00;
    userEeprom[0x1B6] = 0x00;
    userEeprom[0x1B7] = 0x00;
    userEeprom[0x1B8] = 0x00;
    userEeprom[0x1B9] = 0x00;
    userEeprom[0x1BA] = 0x00;
    userEeprom[0x1BB] = 0x00;
    userEeprom[0x1BC] = 0x00;
    userEeprom[0x1BD] = 0x00;
    userEeprom[0x1BE] = 0x00;
    userEeprom[0x1BF] = 0x00;
    userEeprom[0x1C0] = 0x00;
    userEeprom[0x1C1] = 0x00;
    userEeprom[0x1C2] = 0x00;
    userEeprom[0x1C3] = 0x00;
    userEeprom[0x1C4] = 0x00;
    userEeprom[0x1C5] = 0x00;
    userEeprom[0x1C6] = 0x00;
    userEeprom[0x1C7] = 0x00;
    userEeprom[0x1C8] = 0x00;
    userEeprom[0x1C9] = 0x00;
    userEeprom[0x1CA] = 0x00;
    userEeprom[0x1CB] = 0x00;
    userEeprom[0x1CC] = 0x00;
    userEeprom[0x1CD] = 0x00;
    userEeprom[0x1CE] = 0x00;
    userEeprom[0x1CF] = 0x00;
    userEeprom[0x1D0] = 0x00;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x00;
    userEeprom[0x1D3] = 0x00;
    userEeprom[0x1D4] = 0x00;
    userEeprom[0x1D5] = 0x00;
    userEeprom[0x1D6] = 0x00;
    userEeprom[0x1D7] = 0x00;
    userEeprom[0x1D8] = 0x00;
    userEeprom[0x1D9] = 0x00;
    userEeprom[0x1DA] = 0x01;
    userEeprom[0x1DB] = 0x03;
    userEeprom[0x1DC] = 0x00;
    userEeprom[0x1DD] = 0x00;
    userEeprom[0x1DE] = 0x00;
    userEeprom[0x1DF] = 0x00;
    userEeprom[0x1E0] = 0x00;
    userEeprom[0x1E1] = 0x00;
    userEeprom[0x1E2] = 0x02;
    userEeprom[0x1E3] = 0x04;
    userEeprom[0x1E4] = 0x00;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x00;
    userEeprom[0x1E8] = 0x00;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x00;
    userEeprom[0x1EB] = 0x00;
    userEeprom[0x1EC] = 0x00;
    userEeprom[0x1ED] = 0x00;
    userEeprom[0x1EE] = 0x00;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x00;
    userEeprom[0x1F2] = 0x00;
    userEeprom[0x1F3] = 0x00;
    userEeprom[0x1F4] = 0x00;
    userEeprom[0x1F5] = 0x00;
    userEeprom[0x1F6] = 0x00;
    userEeprom[0x1F7] = 0x00;
    userEeprom[0x1F8] = 0x00;
    userEeprom[0x1F9] = 0x00;
    userEeprom[0x1FA] = 0x01;
    userEeprom[0x1FB] = 0x00;
    userEeprom[0x1FC] = 0x00;
    userEeprom[0x1FD] = 0x00;
    userEeprom[0x1FE] = 0x00;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_simple_timeout[] =
{
  {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   1
  // init feedback telegram for output 1
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x80}} //   2
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   3
  // and out 2
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE1, 0x00, 0x80}} //   4
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   5
  // only two feedback channel telegrams
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //   6
  // After the power up the PWM will be enabled
, {TIMER_TICK     , 20,  0, (StepFunction *) _enablePWM          , {}} //   7

  // receive a "ON" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}} //   8
  // process the received telegram inside the app
  // we have a (130*1)*1 on delay -> the output should be switch on in 130ms
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   9
, {TIMER_TICK     , 60,  0, (StepFunction *) _loop               , {}} //  10

  // receive a "ON" telegram for output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}} //  11
  // process the received telegram inside the app
  // we have a (130*2)*3 on delay -> the output should be switch on in 780ms
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  12
, {TIMER_TICK     , 68,  0, (StepFunction *) _loop               , {}} //  13
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Set         , {}} //  14
  // another loop is required for the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  15
  // feedback telegram for output 1 ON
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x81}} //  16
  // only for one feedback channel a group address is store -> only one telegram should be sent
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //  17
, {TIMER_TICK     , 10,  0, (StepFunction *) _enablePWM          , {}} //  18
, {TIMER_TICK     , 699,  0, (StepFunction *) _loop               , {}} //  19
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Set         , {}} //  20
  // another loop is required for the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  21
  // feedback telegram for output 2 ON
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE1, 0x00, 0x81}} //  22
  // only for one feedback channel a group address is store -> only one telegram should be sent
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //  23
, {TIMER_TICK     , 10,  0, (StepFunction *) _enablePWM          , {}} //  24

  // receive a "OFF" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}} //  25
  // process the received telegram inside the app
  // we have a (130*1)*2 on delay -> the output should be switch off in 260ms
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  26
, {TIMER_TICK     , 259,  0, (StepFunction *) _loop               , {}} //  27
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Clear       , {}} //  28
  // another loop is required for the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  29
  // feedback telegram for output 1 OFF
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x80}} //  30
  // only for one feedback channel a group address is store -> only one telegram should be sent
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //  31
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  32
, {END}
};
static Test_Case simple_timeout_tc =
{
  "OUT8 - Timeout Test 1"
, 0x0004, 0x2060, 01
, 0 // power-on delay
, simple_timeout_eepromSetup
, NULL
, (StateFunction *) _gatherState
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_simple_timeout
};

TEST_CASE("OUT8 - Timeout Test 1","[DELAY]")
{
  executeTest(& simple_timeout_tc);
}
// <<< TC:simple_timeout
#endif

#if 1
// >>> TC:timed_function
// Date: 2014-06-27 13:13:34.881024

/* Code for test case timed_function */
static void timed_function_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-06-27 13:13:34.881024
    // Assoc Table (0x15F):
    //    1 ( 1/0/20) <-> 12 (feedback 1          ) @ 0x160
    //    2 ( 1/0/21) <-> 13 (feedback 2          ) @ 0x162
    //    3 ( 1/0/30) <->  0 (output 1            ) @ 0x164
    //    4 ( 1/0/31) <->  1 (output 2            ) @ 0x166
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/20 @ 0x119
    //   ( 2)  1/0/21 @ 0x11B
    //   ( 3)  1/0/30 @ 0x11D
    //   ( 4)  1/0/31 @ 0x11F
    // Com Object table (0x121):
    //   ( 0) output 1             <6B, 17, 00> @ 0x123
    //   ( 1) output 2             <6C, 17, 00> @ 0x126
    //   ( 2) output 3             <6D, 17, 00> @ 0x129
    //   ( 3) output 4             <6E, 17, 00> @ 0x12C
    //   ( 4) output 5             <6F, 17, 00> @ 0x12F
    //   ( 5) output 6             <70, 17, 00> @ 0x132
    //   ( 6) output 7             <71, 17, 00> @ 0x135
    //   ( 7) output 8             <72, 17, 00> @ 0x138
    //   ( 8) special 1            <73, 1F, 01> @ 0x13B
    //   ( 9) special 2            <74, 1F, 01> @ 0x13E
    //   (10) special 3            <75, 1F, 01> @ 0x141
    //   (11) special 4            <76, 1F, 01> @ 0x144
    //   (12) feedback 1           <77, 4F, 00> @ 0x147
    //   (13) feedback 2           <78, 4F, 00> @ 0x14A
    //   (14) feedback 3           <79, 4F, 00> @ 0x14D
    //   (15) feedback 4           <7A, 4F, 00> @ 0x150
    //   (16) feedback 5           <7B, 4F, 00> @ 0x153
    //   (17) feedback 6           <7C, 4F, 00> @ 0x156
    //   (18) feedback 7           <7D, 4F, 00> @ 0x159
    //   (19) feedback 8           <7E, 4F, 00> @ 0x15C
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x60;
    userEeprom[0x106] = 0x20;
    userEeprom[0x107] = 0x01;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x5F;
    userEeprom[0x112] = 0x21;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x04;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x14;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x15;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x1E;
    userEeprom[0x11F] = 0x08;
    userEeprom[0x120] = 0x1F;
    userEeprom[0x121] = 0x14;
    userEeprom[0x122] = 0x61;
    userEeprom[0x123] = 0x6B;
    userEeprom[0x124] = 0x17;
    userEeprom[0x125] = 0x00;
    userEeprom[0x126] = 0x6C;
    userEeprom[0x127] = 0x17;
    userEeprom[0x128] = 0x00;
    userEeprom[0x129] = 0x6D;
    userEeprom[0x12A] = 0x17;
    userEeprom[0x12B] = 0x00;
    userEeprom[0x12C] = 0x6E;
    userEeprom[0x12D] = 0x17;
    userEeprom[0x12E] = 0x00;
    userEeprom[0x12F] = 0x6F;
    userEeprom[0x130] = 0x17;
    userEeprom[0x131] = 0x00;
    userEeprom[0x132] = 0x70;
    userEeprom[0x133] = 0x17;
    userEeprom[0x134] = 0x00;
    userEeprom[0x135] = 0x71;
    userEeprom[0x136] = 0x17;
    userEeprom[0x137] = 0x00;
    userEeprom[0x138] = 0x72;
    userEeprom[0x139] = 0x17;
    userEeprom[0x13A] = 0x00;
    userEeprom[0x13B] = 0x73;
    userEeprom[0x13C] = 0x1F;
    userEeprom[0x13D] = 0x01;
    userEeprom[0x13E] = 0x74;
    userEeprom[0x13F] = 0x1F;
    userEeprom[0x140] = 0x01;
    userEeprom[0x141] = 0x75;
    userEeprom[0x142] = 0x1F;
    userEeprom[0x143] = 0x01;
    userEeprom[0x144] = 0x76;
    userEeprom[0x145] = 0x1F;
    userEeprom[0x146] = 0x01;
    userEeprom[0x147] = 0x77;
    userEeprom[0x148] = 0x4F;
    userEeprom[0x149] = 0x00;
    userEeprom[0x14A] = 0x78;
    userEeprom[0x14B] = 0x4F;
    userEeprom[0x14C] = 0x00;
    userEeprom[0x14D] = 0x79;
    userEeprom[0x14E] = 0x4F;
    userEeprom[0x14F] = 0x00;
    userEeprom[0x150] = 0x7A;
    userEeprom[0x151] = 0x4F;
    userEeprom[0x152] = 0x00;
    userEeprom[0x153] = 0x7B;
    userEeprom[0x154] = 0x4F;
    userEeprom[0x155] = 0x00;
    userEeprom[0x156] = 0x7C;
    userEeprom[0x157] = 0x4F;
    userEeprom[0x158] = 0x00;
    userEeprom[0x159] = 0x7D;
    userEeprom[0x15A] = 0x4F;
    userEeprom[0x15B] = 0x00;
    userEeprom[0x15C] = 0x7E;
    userEeprom[0x15D] = 0x4F;
    userEeprom[0x15E] = 0x00;
    userEeprom[0x15F] = 0x04;
    userEeprom[0x160] = 0x01;
    userEeprom[0x161] = 0x0C;
    userEeprom[0x162] = 0x02;
    userEeprom[0x163] = 0x0D;
    userEeprom[0x164] = 0x03;
    userEeprom[0x165] = 0x00;
    userEeprom[0x166] = 0x04;
    userEeprom[0x167] = 0x01;
    userEeprom[0x168] = 0x00;
    userEeprom[0x169] = 0x00;
    userEeprom[0x16A] = 0x00;
    userEeprom[0x16B] = 0x00;
    userEeprom[0x16C] = 0x00;
    userEeprom[0x16D] = 0x00;
    userEeprom[0x16E] = 0x00;
    userEeprom[0x16F] = 0x00;
    userEeprom[0x170] = 0x00;
    userEeprom[0x171] = 0x00;
    userEeprom[0x172] = 0x00;
    userEeprom[0x173] = 0x00;
    userEeprom[0x174] = 0x00;
    userEeprom[0x175] = 0x00;
    userEeprom[0x176] = 0x00;
    userEeprom[0x177] = 0x00;
    userEeprom[0x178] = 0x00;
    userEeprom[0x179] = 0x00;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x00;
    userEeprom[0x17C] = 0x00;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x00;
    userEeprom[0x17F] = 0x00;
    userEeprom[0x180] = 0x00;
    userEeprom[0x181] = 0x00;
    userEeprom[0x182] = 0x00;
    userEeprom[0x183] = 0x00;
    userEeprom[0x184] = 0x00;
    userEeprom[0x185] = 0x00;
    userEeprom[0x186] = 0x00;
    userEeprom[0x187] = 0x00;
    userEeprom[0x188] = 0x00;
    userEeprom[0x189] = 0x00;
    userEeprom[0x18A] = 0x00;
    userEeprom[0x18B] = 0x00;
    userEeprom[0x18C] = 0x00;
    userEeprom[0x18D] = 0x00;
    userEeprom[0x18E] = 0x00;
    userEeprom[0x18F] = 0x00;
    userEeprom[0x190] = 0x00;
    userEeprom[0x191] = 0x00;
    userEeprom[0x192] = 0x00;
    userEeprom[0x193] = 0x00;
    userEeprom[0x194] = 0x00;
    userEeprom[0x195] = 0x00;
    userEeprom[0x196] = 0x00;
    userEeprom[0x197] = 0x00;
    userEeprom[0x198] = 0x00;
    userEeprom[0x199] = 0x00;
    userEeprom[0x19A] = 0x00;
    userEeprom[0x19B] = 0x00;
    userEeprom[0x19C] = 0x00;
    userEeprom[0x19D] = 0x00;
    userEeprom[0x19E] = 0x00;
    userEeprom[0x19F] = 0x00;
    userEeprom[0x1A0] = 0x00;
    userEeprom[0x1A1] = 0x00;
    userEeprom[0x1A2] = 0x00;
    userEeprom[0x1A3] = 0x00;
    userEeprom[0x1A4] = 0x00;
    userEeprom[0x1A5] = 0x00;
    userEeprom[0x1A6] = 0x00;
    userEeprom[0x1A7] = 0x00;
    userEeprom[0x1A8] = 0x00;
    userEeprom[0x1A9] = 0x00;
    userEeprom[0x1AA] = 0x00;
    userEeprom[0x1AB] = 0x00;
    userEeprom[0x1AC] = 0x00;
    userEeprom[0x1AD] = 0x00;
    userEeprom[0x1AE] = 0x00;
    userEeprom[0x1AF] = 0x00;
    userEeprom[0x1B0] = 0x00;
    userEeprom[0x1B1] = 0x00;
    userEeprom[0x1B2] = 0x00;
    userEeprom[0x1B3] = 0x00;
    userEeprom[0x1B4] = 0x00;
    userEeprom[0x1B5] = 0x00;
    userEeprom[0x1B6] = 0x00;
    userEeprom[0x1B7] = 0x00;
    userEeprom[0x1B8] = 0x00;
    userEeprom[0x1B9] = 0x00;
    userEeprom[0x1BA] = 0x00;
    userEeprom[0x1BB] = 0x00;
    userEeprom[0x1BC] = 0x00;
    userEeprom[0x1BD] = 0x00;
    userEeprom[0x1BE] = 0x00;
    userEeprom[0x1BF] = 0x00;
    userEeprom[0x1C0] = 0x00;
    userEeprom[0x1C1] = 0x00;
    userEeprom[0x1C2] = 0x00;
    userEeprom[0x1C3] = 0x00;
    userEeprom[0x1C4] = 0x00;
    userEeprom[0x1C5] = 0x00;
    userEeprom[0x1C6] = 0x00;
    userEeprom[0x1C7] = 0x00;
    userEeprom[0x1C8] = 0x00;
    userEeprom[0x1C9] = 0x00;
    userEeprom[0x1CA] = 0x00;
    userEeprom[0x1CB] = 0x00;
    userEeprom[0x1CC] = 0x00;
    userEeprom[0x1CD] = 0x00;
    userEeprom[0x1CE] = 0x00;
    userEeprom[0x1CF] = 0x00;
    userEeprom[0x1D0] = 0x00;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x00;
    userEeprom[0x1D3] = 0x00;
    userEeprom[0x1D4] = 0x00;
    userEeprom[0x1D5] = 0x00;
    userEeprom[0x1D6] = 0x00;
    userEeprom[0x1D7] = 0x00;
    userEeprom[0x1D8] = 0x00;
    userEeprom[0x1D9] = 0x00;
    userEeprom[0x1DA] = 0x00;
    userEeprom[0x1DB] = 0x02;
    userEeprom[0x1DC] = 0x00;
    userEeprom[0x1DD] = 0x00;
    userEeprom[0x1DE] = 0x00;
    userEeprom[0x1DF] = 0x00;
    userEeprom[0x1E0] = 0x00;
    userEeprom[0x1E1] = 0x00;
    userEeprom[0x1E2] = 0x01;
    userEeprom[0x1E3] = 0x03;
    userEeprom[0x1E4] = 0x00;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x00;
    userEeprom[0x1E8] = 0x00;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x03;
    userEeprom[0x1EB] = 0x01;
    userEeprom[0x1EC] = 0x00;
    userEeprom[0x1ED] = 0x00;
    userEeprom[0x1EE] = 0x00;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x00;
    userEeprom[0x1F2] = 0x00;
    userEeprom[0x1F3] = 0x00;
    userEeprom[0x1F4] = 0x00;
    userEeprom[0x1F5] = 0x00;
    userEeprom[0x1F6] = 0x00;
    userEeprom[0x1F7] = 0x00;
    userEeprom[0x1F8] = 0x00;
    userEeprom[0x1F9] = 0x00;
    userEeprom[0x1FA] = 0x01;
    userEeprom[0x1FB] = 0x00;
    userEeprom[0x1FC] = 0x00;
    userEeprom[0x1FD] = 0x00;
    userEeprom[0x1FE] = 0x00;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_timed_function[] =
{
  {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   1
  // init feedback for out 1
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x80}} //   2
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   3
  // and out 2
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE1, 0x00, 0x80}} //   4
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   5
  // only two feedback channel telegrams
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //   6
  // After the power up the PWM will be enabled
, {TIMER_TICK     , 20,  0, (StepFunction *) _enablePWM          , {}} //   7

  // receive a "ON" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}} //   8
  // process the received telegram inside the app
  // we have a (130*1)*1 on delay -> the output should be switch on in 130ms
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Set         , {}} //   9
  // another app loop is required for the the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  10
  // as a reaction to the change a feedback telegram should be sent
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x81}} //  11
, {TIMER_TICK     , 10,  0, (StepFunction *) _enablePWM          , {}} //  12
, {TIMER_TICK     , 118,  0, (StepFunction *) _loop               , {}} //  13
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Clear       , {}} //  14
  // another app loop is required for the the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  15
  // as a reaction to the change a feedback telegram should be sent
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x80}} //  16

  // receive a "ON" telegram for output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}} //  17
  // process the received telegram inside the app
  // we have a (130*2)*2 on delay -> the output should be switch on in 520ms
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  18
  // wait for 520ms before the output will be switched on
, {TIMER_TICK     , 519,  0, (StepFunction *) _loop               , {}} //  19
  // 520ms have gone by -> output will be switched on
  // and a off delay off (130*2)*3=780ms will be started
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Set         , {}} //  20
  // another app loop is required for the the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  21
  // as a reaction to the change a feedback telegram should be sent
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE1, 0x00, 0x81}} //  22
, {TIMER_TICK     , 10,  0, (StepFunction *) _enablePWM          , {}} //  23

  // receive a "ON" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}} //  24
  // process the received telegram inside the app
  // we have a (130*2)*2 on delay -> the output should be switch on in 520ms
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Set         , {}} //  25
  // another app loop is required for the the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  26
  // as a reaction to the change a feedback telegram should be sent
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x81}} //  27
, {TIMER_TICK     , 10,  0, (StepFunction *) _enablePWM          , {}} //  28

  // receive a "OFF" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}} //  29
  // An off telegram has beed received for output 1
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  30

  // receive a "OFF" telegram for output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}} //  31
  // An off telegram has beed received for output 2
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Clear       , {}} //  32
  // Another tick to create the feedback telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  33
  // as a reaction to the change a feedback telegram should be sent
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE1, 0x00, 0x80}} //  34
, {TIMER_TICK     , 115,  0, (StepFunction *) _loop               , {}} //  35
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Clear       , {}} //  36
  // another app loop is required for the the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  37
  // as a reaction to the change a feedback telegram should be sent
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x80}} //  38
, {END}
};
static Test_Case timed_function_tc =
{
  "OUT8 - Timed Test 1"
, 0x0004, 0x2060, 01
, 0 // power-on delay
, timed_function_eepromSetup
, NULL
, (StateFunction *) _gatherState
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_timed_function
};

TEST_CASE("OUT8 - Timed Test 1","[TIMED]")
{
  executeTest(& timed_function_tc);
}
// <<< TC:timed_function
#endif

#if 1
// >>> TC:simple_i
// Date: 2014-06-27 13:13:34.920026

/* Code for test case simple_i */
static void simple_i_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-06-27 13:13:34.921026
    // Assoc Table (0x15F):
    //    1 ( 1/0/20) <-> 12 (feedback 1          ) @ 0x160
    //    2 ( 1/0/21) <-> 13 (feedback 2          ) @ 0x162
    //    3 ( 1/0/30) <->  0 (output 1            ) @ 0x164
    //    4 ( 1/0/31) <->  1 (output 2            ) @ 0x166
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/20 @ 0x119
    //   ( 2)  1/0/21 @ 0x11B
    //   ( 3)  1/0/30 @ 0x11D
    //   ( 4)  1/0/31 @ 0x11F
    // Com Object table (0x121):
    //   ( 0) output 1             <6B, 17, 00> @ 0x123
    //   ( 1) output 2             <6C, 17, 00> @ 0x126
    //   ( 2) output 3             <6D, 17, 00> @ 0x129
    //   ( 3) output 4             <6E, 17, 00> @ 0x12C
    //   ( 4) output 5             <6F, 17, 00> @ 0x12F
    //   ( 5) output 6             <70, 17, 00> @ 0x132
    //   ( 6) output 7             <71, 17, 00> @ 0x135
    //   ( 7) output 8             <72, 17, 00> @ 0x138
    //   ( 8) special 1            <73, 1F, 01> @ 0x13B
    //   ( 9) special 2            <74, 1F, 01> @ 0x13E
    //   (10) special 3            <75, 1F, 01> @ 0x141
    //   (11) special 4            <76, 1F, 01> @ 0x144
    //   (12) feedback 1           <77, 4F, 00> @ 0x147
    //   (13) feedback 2           <78, 4F, 00> @ 0x14A
    //   (14) feedback 3           <79, 4F, 00> @ 0x14D
    //   (15) feedback 4           <7A, 4F, 00> @ 0x150
    //   (16) feedback 5           <7B, 4F, 00> @ 0x153
    //   (17) feedback 6           <7C, 4F, 00> @ 0x156
    //   (18) feedback 7           <7D, 4F, 00> @ 0x159
    //   (19) feedback 8           <7E, 4F, 00> @ 0x15C
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x60;
    userEeprom[0x106] = 0x20;
    userEeprom[0x107] = 0x01;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x5F;
    userEeprom[0x112] = 0x21;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x04;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x14;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x15;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x1E;
    userEeprom[0x11F] = 0x08;
    userEeprom[0x120] = 0x1F;
    userEeprom[0x121] = 0x14;
    userEeprom[0x122] = 0x61;
    userEeprom[0x123] = 0x6B;
    userEeprom[0x124] = 0x17;
    userEeprom[0x125] = 0x00;
    userEeprom[0x126] = 0x6C;
    userEeprom[0x127] = 0x17;
    userEeprom[0x128] = 0x00;
    userEeprom[0x129] = 0x6D;
    userEeprom[0x12A] = 0x17;
    userEeprom[0x12B] = 0x00;
    userEeprom[0x12C] = 0x6E;
    userEeprom[0x12D] = 0x17;
    userEeprom[0x12E] = 0x00;
    userEeprom[0x12F] = 0x6F;
    userEeprom[0x130] = 0x17;
    userEeprom[0x131] = 0x00;
    userEeprom[0x132] = 0x70;
    userEeprom[0x133] = 0x17;
    userEeprom[0x134] = 0x00;
    userEeprom[0x135] = 0x71;
    userEeprom[0x136] = 0x17;
    userEeprom[0x137] = 0x00;
    userEeprom[0x138] = 0x72;
    userEeprom[0x139] = 0x17;
    userEeprom[0x13A] = 0x00;
    userEeprom[0x13B] = 0x73;
    userEeprom[0x13C] = 0x1F;
    userEeprom[0x13D] = 0x01;
    userEeprom[0x13E] = 0x74;
    userEeprom[0x13F] = 0x1F;
    userEeprom[0x140] = 0x01;
    userEeprom[0x141] = 0x75;
    userEeprom[0x142] = 0x1F;
    userEeprom[0x143] = 0x01;
    userEeprom[0x144] = 0x76;
    userEeprom[0x145] = 0x1F;
    userEeprom[0x146] = 0x01;
    userEeprom[0x147] = 0x77;
    userEeprom[0x148] = 0x4F;
    userEeprom[0x149] = 0x00;
    userEeprom[0x14A] = 0x78;
    userEeprom[0x14B] = 0x4F;
    userEeprom[0x14C] = 0x00;
    userEeprom[0x14D] = 0x79;
    userEeprom[0x14E] = 0x4F;
    userEeprom[0x14F] = 0x00;
    userEeprom[0x150] = 0x7A;
    userEeprom[0x151] = 0x4F;
    userEeprom[0x152] = 0x00;
    userEeprom[0x153] = 0x7B;
    userEeprom[0x154] = 0x4F;
    userEeprom[0x155] = 0x00;
    userEeprom[0x156] = 0x7C;
    userEeprom[0x157] = 0x4F;
    userEeprom[0x158] = 0x00;
    userEeprom[0x159] = 0x7D;
    userEeprom[0x15A] = 0x4F;
    userEeprom[0x15B] = 0x00;
    userEeprom[0x15C] = 0x7E;
    userEeprom[0x15D] = 0x4F;
    userEeprom[0x15E] = 0x00;
    userEeprom[0x15F] = 0x04;
    userEeprom[0x160] = 0x01;
    userEeprom[0x161] = 0x0C;
    userEeprom[0x162] = 0x02;
    userEeprom[0x163] = 0x0D;
    userEeprom[0x164] = 0x03;
    userEeprom[0x165] = 0x00;
    userEeprom[0x166] = 0x04;
    userEeprom[0x167] = 0x01;
    userEeprom[0x168] = 0x00;
    userEeprom[0x169] = 0x00;
    userEeprom[0x16A] = 0x00;
    userEeprom[0x16B] = 0x00;
    userEeprom[0x16C] = 0x00;
    userEeprom[0x16D] = 0x00;
    userEeprom[0x16E] = 0x00;
    userEeprom[0x16F] = 0x00;
    userEeprom[0x170] = 0x00;
    userEeprom[0x171] = 0x00;
    userEeprom[0x172] = 0x00;
    userEeprom[0x173] = 0x00;
    userEeprom[0x174] = 0x00;
    userEeprom[0x175] = 0x00;
    userEeprom[0x176] = 0x00;
    userEeprom[0x177] = 0x00;
    userEeprom[0x178] = 0x00;
    userEeprom[0x179] = 0x00;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x00;
    userEeprom[0x17C] = 0x00;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x00;
    userEeprom[0x17F] = 0x00;
    userEeprom[0x180] = 0x00;
    userEeprom[0x181] = 0x00;
    userEeprom[0x182] = 0x00;
    userEeprom[0x183] = 0x00;
    userEeprom[0x184] = 0x00;
    userEeprom[0x185] = 0x00;
    userEeprom[0x186] = 0x00;
    userEeprom[0x187] = 0x00;
    userEeprom[0x188] = 0x00;
    userEeprom[0x189] = 0x00;
    userEeprom[0x18A] = 0x00;
    userEeprom[0x18B] = 0x00;
    userEeprom[0x18C] = 0x00;
    userEeprom[0x18D] = 0x00;
    userEeprom[0x18E] = 0x00;
    userEeprom[0x18F] = 0x00;
    userEeprom[0x190] = 0x00;
    userEeprom[0x191] = 0x00;
    userEeprom[0x192] = 0x00;
    userEeprom[0x193] = 0x00;
    userEeprom[0x194] = 0x00;
    userEeprom[0x195] = 0x00;
    userEeprom[0x196] = 0x00;
    userEeprom[0x197] = 0x00;
    userEeprom[0x198] = 0x00;
    userEeprom[0x199] = 0x00;
    userEeprom[0x19A] = 0x00;
    userEeprom[0x19B] = 0x00;
    userEeprom[0x19C] = 0x00;
    userEeprom[0x19D] = 0x00;
    userEeprom[0x19E] = 0x00;
    userEeprom[0x19F] = 0x00;
    userEeprom[0x1A0] = 0x00;
    userEeprom[0x1A1] = 0x00;
    userEeprom[0x1A2] = 0x00;
    userEeprom[0x1A3] = 0x00;
    userEeprom[0x1A4] = 0x00;
    userEeprom[0x1A5] = 0x00;
    userEeprom[0x1A6] = 0x00;
    userEeprom[0x1A7] = 0x00;
    userEeprom[0x1A8] = 0x00;
    userEeprom[0x1A9] = 0x00;
    userEeprom[0x1AA] = 0x00;
    userEeprom[0x1AB] = 0x00;
    userEeprom[0x1AC] = 0x00;
    userEeprom[0x1AD] = 0x00;
    userEeprom[0x1AE] = 0x00;
    userEeprom[0x1AF] = 0x00;
    userEeprom[0x1B0] = 0x00;
    userEeprom[0x1B1] = 0x00;
    userEeprom[0x1B2] = 0x00;
    userEeprom[0x1B3] = 0x00;
    userEeprom[0x1B4] = 0x00;
    userEeprom[0x1B5] = 0x00;
    userEeprom[0x1B6] = 0x00;
    userEeprom[0x1B7] = 0x00;
    userEeprom[0x1B8] = 0x00;
    userEeprom[0x1B9] = 0x00;
    userEeprom[0x1BA] = 0x00;
    userEeprom[0x1BB] = 0x00;
    userEeprom[0x1BC] = 0x00;
    userEeprom[0x1BD] = 0x00;
    userEeprom[0x1BE] = 0x00;
    userEeprom[0x1BF] = 0x00;
    userEeprom[0x1C0] = 0x00;
    userEeprom[0x1C1] = 0x00;
    userEeprom[0x1C2] = 0x00;
    userEeprom[0x1C3] = 0x00;
    userEeprom[0x1C4] = 0x00;
    userEeprom[0x1C5] = 0x00;
    userEeprom[0x1C6] = 0x00;
    userEeprom[0x1C7] = 0x00;
    userEeprom[0x1C8] = 0x00;
    userEeprom[0x1C9] = 0x00;
    userEeprom[0x1CA] = 0x00;
    userEeprom[0x1CB] = 0x00;
    userEeprom[0x1CC] = 0x00;
    userEeprom[0x1CD] = 0x00;
    userEeprom[0x1CE] = 0x00;
    userEeprom[0x1CF] = 0x00;
    userEeprom[0x1D0] = 0x00;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x00;
    userEeprom[0x1D3] = 0x00;
    userEeprom[0x1D4] = 0x00;
    userEeprom[0x1D5] = 0x00;
    userEeprom[0x1D6] = 0x00;
    userEeprom[0x1D7] = 0x00;
    userEeprom[0x1D8] = 0x00;
    userEeprom[0x1D9] = 0x00;
    userEeprom[0x1DA] = 0x00;
    userEeprom[0x1DB] = 0x00;
    userEeprom[0x1DC] = 0x00;
    userEeprom[0x1DD] = 0x00;
    userEeprom[0x1DE] = 0x00;
    userEeprom[0x1DF] = 0x00;
    userEeprom[0x1E0] = 0x00;
    userEeprom[0x1E1] = 0x00;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x00;
    userEeprom[0x1E4] = 0x00;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x00;
    userEeprom[0x1E8] = 0x00;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x00;
    userEeprom[0x1EB] = 0x00;
    userEeprom[0x1EC] = 0x00;
    userEeprom[0x1ED] = 0x00;
    userEeprom[0x1EE] = 0x00;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x00;
    userEeprom[0x1F2] = 0x02;
    userEeprom[0x1F3] = 0x00;
    userEeprom[0x1F4] = 0x00;
    userEeprom[0x1F5] = 0x00;
    userEeprom[0x1F6] = 0x00;
    userEeprom[0x1F7] = 0x00;
    userEeprom[0x1F8] = 0x00;
    userEeprom[0x1F9] = 0x00;
    userEeprom[0x1FA] = 0x00;
    userEeprom[0x1FB] = 0x00;
    userEeprom[0x1FC] = 0x00;
    userEeprom[0x1FD] = 0x00;
    userEeprom[0x1FE] = 0x00;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_simple_i[] =
{
  {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   1
  // feedback output 1
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x80}} //   2
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   3
  // feedback output 2
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE1, 0x00, 0x80}} //   4
  // only two feedback telegrams
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //   5

  // receive a "ON" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}} //   6
  // process the received telegram inside the app
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Set         , {}} //   7
  // another loop is required for the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   8
  // feedback telegram ON for output 1
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x81}} //   9

  // receive a "ON" telegram for output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}} //  10
  // process the received telegram inside the app
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Clear       , {}} //  11
  // another loop is required for the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  12
  // feedback telegram ON for output 1
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE1, 0x00, 0x81}} //  13

  // receive a "OFF" telegram for output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}} //  14
  // process the received telegram inside the app
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Set         , {}} //  15
  // another loop is required for the feedback telegram creation
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  16
  // feedback telegram OFF for output 2
, {TEL_TX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE1, 0x00, 0x80}} //  17
  // only for one feedback channel a group address is store -> only one telegram should be sent
, {CHECK_TX_BUFFER,  0,  0, (StepFunction *) NULL                , {}} //  18
, {TIMER_TICK     , 10,  0, (StepFunction *) _enablePWM          , {}} //  19
, {END}
};
static Test_Case simple_i_tc =
{
  "OUT8 - Test Inverted"
, 0x0004, 0x2060, 01
, 0 // power-on delay
, simple_i_eepromSetup
, NULL
, (StateFunction *) _gatherState
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_simple_i
};

TEST_CASE("OUT8 - Test Inverted","[SIMPLE_I]")
{
  executeTest(& simple_i_tc);
}
// <<< TC:simple_i
#endif

#if 1
// >>> TC:special_1
// Date: 2014-06-27 13:13:34.958028

/* Code for test case special_1 */
static void special_1_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-06-27 13:13:34.958028
    // Assoc Table (0x167):
    //    1 ( 1/0/30) <->  0 (output 1            ) @ 0x168
    //    2 ( 1/0/31) <->  1 (output 2            ) @ 0x16A
    //    3 ( 1/0/32) <->  2 (output 3            ) @ 0x16C
    //    4 ( 1/0/33) <->  3 (output 4            ) @ 0x16E
    //    5 ( 1/0/40) <->  8 (special 1           ) @ 0x170
    //    6 ( 1/0/41) <->  9 (special 2           ) @ 0x172
    //    7 ( 1/0/42) <-> 10 (special 3           ) @ 0x174
    //    8 ( 1/0/43) <-> 11 (special 4           ) @ 0x176
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/30 @ 0x119
    //   ( 2)  1/0/31 @ 0x11B
    //   ( 3)  1/0/32 @ 0x11D
    //   ( 4)  1/0/33 @ 0x11F
    //   ( 5)  1/0/40 @ 0x121
    //   ( 6)  1/0/41 @ 0x123
    //   ( 7)  1/0/42 @ 0x125
    //   ( 8)  1/0/43 @ 0x127
    // Com Object table (0x129):
    //   ( 0) output 1             <6B, 17, 00> @ 0x12B
    //   ( 1) output 2             <6C, 17, 00> @ 0x12E
    //   ( 2) output 3             <6D, 17, 00> @ 0x131
    //   ( 3) output 4             <6E, 17, 00> @ 0x134
    //   ( 4) output 5             <6F, 17, 00> @ 0x137
    //   ( 5) output 6             <70, 17, 00> @ 0x13A
    //   ( 6) output 7             <71, 17, 00> @ 0x13D
    //   ( 7) output 8             <72, 17, 00> @ 0x140
    //   ( 8) special 1            <73, 1F, 01> @ 0x143
    //   ( 9) special 2            <74, 1F, 01> @ 0x146
    //   (10) special 3            <75, 1F, 01> @ 0x149
    //   (11) special 4            <76, 1F, 01> @ 0x14C
    //   (12) feedback 1           <77, 4F, 00> @ 0x14F
    //   (13) feedback 2           <78, 4F, 00> @ 0x152
    //   (14) feedback 3           <79, 4F, 00> @ 0x155
    //   (15) feedback 4           <7A, 4F, 00> @ 0x158
    //   (16) feedback 5           <7B, 4F, 00> @ 0x15B
    //   (17) feedback 6           <7C, 4F, 00> @ 0x15E
    //   (18) feedback 7           <7D, 4F, 00> @ 0x161
    //   (19) feedback 8           <7E, 4F, 00> @ 0x164
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x60;
    userEeprom[0x106] = 0x20;
    userEeprom[0x107] = 0x01;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x67;
    userEeprom[0x112] = 0x29;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x08;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x1E;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x1F;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x20;
    userEeprom[0x11F] = 0x08;
    userEeprom[0x120] = 0x21;
    userEeprom[0x121] = 0x08;
    userEeprom[0x122] = 0x28;
    userEeprom[0x123] = 0x08;
    userEeprom[0x124] = 0x29;
    userEeprom[0x125] = 0x08;
    userEeprom[0x126] = 0x2A;
    userEeprom[0x127] = 0x08;
    userEeprom[0x128] = 0x2B;
    userEeprom[0x129] = 0x14;
    userEeprom[0x12A] = 0x61;
    userEeprom[0x12B] = 0x6B;
    userEeprom[0x12C] = 0x17;
    userEeprom[0x12D] = 0x00;
    userEeprom[0x12E] = 0x6C;
    userEeprom[0x12F] = 0x17;
    userEeprom[0x130] = 0x00;
    userEeprom[0x131] = 0x6D;
    userEeprom[0x132] = 0x17;
    userEeprom[0x133] = 0x00;
    userEeprom[0x134] = 0x6E;
    userEeprom[0x135] = 0x17;
    userEeprom[0x136] = 0x00;
    userEeprom[0x137] = 0x6F;
    userEeprom[0x138] = 0x17;
    userEeprom[0x139] = 0x00;
    userEeprom[0x13A] = 0x70;
    userEeprom[0x13B] = 0x17;
    userEeprom[0x13C] = 0x00;
    userEeprom[0x13D] = 0x71;
    userEeprom[0x13E] = 0x17;
    userEeprom[0x13F] = 0x00;
    userEeprom[0x140] = 0x72;
    userEeprom[0x141] = 0x17;
    userEeprom[0x142] = 0x00;
    userEeprom[0x143] = 0x73;
    userEeprom[0x144] = 0x1F;
    userEeprom[0x145] = 0x01;
    userEeprom[0x146] = 0x74;
    userEeprom[0x147] = 0x1F;
    userEeprom[0x148] = 0x01;
    userEeprom[0x149] = 0x75;
    userEeprom[0x14A] = 0x1F;
    userEeprom[0x14B] = 0x01;
    userEeprom[0x14C] = 0x76;
    userEeprom[0x14D] = 0x1F;
    userEeprom[0x14E] = 0x01;
    userEeprom[0x14F] = 0x77;
    userEeprom[0x150] = 0x4F;
    userEeprom[0x151] = 0x00;
    userEeprom[0x152] = 0x78;
    userEeprom[0x153] = 0x4F;
    userEeprom[0x154] = 0x00;
    userEeprom[0x155] = 0x79;
    userEeprom[0x156] = 0x4F;
    userEeprom[0x157] = 0x00;
    userEeprom[0x158] = 0x7A;
    userEeprom[0x159] = 0x4F;
    userEeprom[0x15A] = 0x00;
    userEeprom[0x15B] = 0x7B;
    userEeprom[0x15C] = 0x4F;
    userEeprom[0x15D] = 0x00;
    userEeprom[0x15E] = 0x7C;
    userEeprom[0x15F] = 0x4F;
    userEeprom[0x160] = 0x00;
    userEeprom[0x161] = 0x7D;
    userEeprom[0x162] = 0x4F;
    userEeprom[0x163] = 0x00;
    userEeprom[0x164] = 0x7E;
    userEeprom[0x165] = 0x4F;
    userEeprom[0x166] = 0x00;
    userEeprom[0x167] = 0x08;
    userEeprom[0x168] = 0x01;
    userEeprom[0x169] = 0x00;
    userEeprom[0x16A] = 0x02;
    userEeprom[0x16B] = 0x01;
    userEeprom[0x16C] = 0x03;
    userEeprom[0x16D] = 0x02;
    userEeprom[0x16E] = 0x04;
    userEeprom[0x16F] = 0x03;
    userEeprom[0x170] = 0x05;
    userEeprom[0x171] = 0x08;
    userEeprom[0x172] = 0x06;
    userEeprom[0x173] = 0x09;
    userEeprom[0x174] = 0x07;
    userEeprom[0x175] = 0x0A;
    userEeprom[0x176] = 0x08;
    userEeprom[0x177] = 0x0B;
    userEeprom[0x178] = 0x00;
    userEeprom[0x179] = 0x00;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x00;
    userEeprom[0x17C] = 0x00;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x00;
    userEeprom[0x17F] = 0x00;
    userEeprom[0x180] = 0x00;
    userEeprom[0x181] = 0x00;
    userEeprom[0x182] = 0x00;
    userEeprom[0x183] = 0x00;
    userEeprom[0x184] = 0x00;
    userEeprom[0x185] = 0x00;
    userEeprom[0x186] = 0x00;
    userEeprom[0x187] = 0x00;
    userEeprom[0x188] = 0x00;
    userEeprom[0x189] = 0x00;
    userEeprom[0x18A] = 0x00;
    userEeprom[0x18B] = 0x00;
    userEeprom[0x18C] = 0x00;
    userEeprom[0x18D] = 0x00;
    userEeprom[0x18E] = 0x00;
    userEeprom[0x18F] = 0x00;
    userEeprom[0x190] = 0x00;
    userEeprom[0x191] = 0x00;
    userEeprom[0x192] = 0x00;
    userEeprom[0x193] = 0x00;
    userEeprom[0x194] = 0x00;
    userEeprom[0x195] = 0x00;
    userEeprom[0x196] = 0x00;
    userEeprom[0x197] = 0x00;
    userEeprom[0x198] = 0x00;
    userEeprom[0x199] = 0x00;
    userEeprom[0x19A] = 0x00;
    userEeprom[0x19B] = 0x00;
    userEeprom[0x19C] = 0x00;
    userEeprom[0x19D] = 0x00;
    userEeprom[0x19E] = 0x00;
    userEeprom[0x19F] = 0x00;
    userEeprom[0x1A0] = 0x00;
    userEeprom[0x1A1] = 0x00;
    userEeprom[0x1A2] = 0x00;
    userEeprom[0x1A3] = 0x00;
    userEeprom[0x1A4] = 0x00;
    userEeprom[0x1A5] = 0x00;
    userEeprom[0x1A6] = 0x00;
    userEeprom[0x1A7] = 0x00;
    userEeprom[0x1A8] = 0x00;
    userEeprom[0x1A9] = 0x00;
    userEeprom[0x1AA] = 0x00;
    userEeprom[0x1AB] = 0x00;
    userEeprom[0x1AC] = 0x00;
    userEeprom[0x1AD] = 0x00;
    userEeprom[0x1AE] = 0x00;
    userEeprom[0x1AF] = 0x00;
    userEeprom[0x1B0] = 0x00;
    userEeprom[0x1B1] = 0x00;
    userEeprom[0x1B2] = 0x00;
    userEeprom[0x1B3] = 0x00;
    userEeprom[0x1B4] = 0x00;
    userEeprom[0x1B5] = 0x00;
    userEeprom[0x1B6] = 0x00;
    userEeprom[0x1B7] = 0x00;
    userEeprom[0x1B8] = 0x00;
    userEeprom[0x1B9] = 0x00;
    userEeprom[0x1BA] = 0x00;
    userEeprom[0x1BB] = 0x00;
    userEeprom[0x1BC] = 0x00;
    userEeprom[0x1BD] = 0x00;
    userEeprom[0x1BE] = 0x00;
    userEeprom[0x1BF] = 0x00;
    userEeprom[0x1C0] = 0x00;
    userEeprom[0x1C1] = 0x00;
    userEeprom[0x1C2] = 0x00;
    userEeprom[0x1C3] = 0x00;
    userEeprom[0x1C4] = 0x00;
    userEeprom[0x1C5] = 0x00;
    userEeprom[0x1C6] = 0x00;
    userEeprom[0x1C7] = 0x00;
    userEeprom[0x1C8] = 0x00;
    userEeprom[0x1C9] = 0x00;
    userEeprom[0x1CA] = 0x00;
    userEeprom[0x1CB] = 0x00;
    userEeprom[0x1CC] = 0x00;
    userEeprom[0x1CD] = 0x00;
    userEeprom[0x1CE] = 0x00;
    userEeprom[0x1CF] = 0x00;
    userEeprom[0x1D0] = 0x00;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x00;
    userEeprom[0x1D3] = 0x00;
    userEeprom[0x1D4] = 0x00;
    userEeprom[0x1D5] = 0x00;
    userEeprom[0x1D6] = 0x00;
    userEeprom[0x1D7] = 0x00;
    userEeprom[0x1D8] = 0x21;
    userEeprom[0x1D9] = 0x43;
    userEeprom[0x1DA] = 0x00;
    userEeprom[0x1DB] = 0x00;
    userEeprom[0x1DC] = 0x00;
    userEeprom[0x1DD] = 0x00;
    userEeprom[0x1DE] = 0x00;
    userEeprom[0x1DF] = 0x00;
    userEeprom[0x1E0] = 0x00;
    userEeprom[0x1E1] = 0x00;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x00;
    userEeprom[0x1E4] = 0x00;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x00;
    userEeprom[0x1E8] = 0x00;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x00;
    userEeprom[0x1EB] = 0x00;
    userEeprom[0x1EC] = 0x00;
    userEeprom[0x1ED] = 0x80;
    userEeprom[0x1EE] = 0x39;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x00;
    userEeprom[0x1F2] = 0x00;
    userEeprom[0x1F3] = 0x00;
    userEeprom[0x1F4] = 0x00;
    userEeprom[0x1F5] = 0x00;
    userEeprom[0x1F6] = 0x00;
    userEeprom[0x1F7] = 0x00;
    userEeprom[0x1F8] = 0x00;
    userEeprom[0x1F9] = 0x00;
    userEeprom[0x1FA] = 0x00;
    userEeprom[0x1FB] = 0x00;
    userEeprom[0x1FC] = 0x00;
    userEeprom[0x1FD] = 0x00;
    userEeprom[0x1FE] = 0x00;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_special_1[] =
{
  {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //   1
  // After the power up the PWM will be enabled
, {TIMER_TICK     , 20,  0, (StepFunction *) _enablePWM          , {}} //   2

  // receive a "ON" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}} //   3
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Set         , {}} //   4
  // receive a "OFF" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}} //   5
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Clear       , {}} //   6

  // receive a "ON" telegram for special 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}} //   7
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Set         , {}} //   8
  // receive a "FF" telegram for special 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}} //   9
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Clear       , {}} //  10

  // receive a "ON" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}} //  11
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Set         , {}} //  12
  // receive a "ON" telegram for special 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}} //  13
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  14
  // receive a "OFF" telegram for output 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}} //  15
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  16
  // receive a "FF" telegram for special 1
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}} //  17
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output1Clear       , {}} //  18

  // receive a "ON" telegram for output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}} //  19
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  20
  // receive a "ON" telegram for special 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x81}} //  21
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Set         , {}} //  22
  // receive a "OFF" telegram for special 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x80}} //  23
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Clear       , {}} //  24
  // receive a "ON" telegram for special 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x81}} //  25
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Set         , {}} //  26
  // receive a "OFF" telegram for output 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}} //  27
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output2Clear       , {}} //  28
  // receive a "OFF" telegram for special 2
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x80}} //  29
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  30

  // AND with recirculation
  // receive a "ON" telegram for output 3
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}} //  31
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  32
  // receive a "ON" telegram for special 3
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x81}} //  33
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output3Set         , {}} //  34
  // receive a "OFF" telegram for special 3
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x80}} //  35
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output3Clear       , {}} //  36
  // process the changed object state of output 3
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  37
  // receive a "ON" telegram for special 3
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x81}} //  38
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  39
  // receive a "ON" telegram for output 3
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}} //  40
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output3Set         , {}} //  41

  // Forced channel 4
  // receive a "ON" telegram for output 4
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x81}} //  42
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output4Set         , {}} //  43
  // receive a 00 telegram for special 4
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x80}} //  44
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  45
  // receive a 01 telegram for special 4
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x81}} //  46
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _loop               , {}} //  47
  // receive a OFF telegram for output 4
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x80}} //  48
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output4Clear       , {}} //  49
  // receive a "ON" telegram for output 4
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x81}} //  50
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output4Set         , {}} //  51

  // receive a 10 telegram for special 4
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x82}} //  52
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output4Clear       , {}} //  53
  // receive a 11 telegram for special 4
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x83}} //  54
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output4Set         , {}} //  55

  // receive a 10 telegram for special 4
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x82}} //  56
  // process the received telegram
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output4Clear       , {}} //  57
  // receive a 10 telegram for special 4 to deactivate the forced value
, {TEL_RX         ,  8,  0, (StepFunction *) NULL                , {0xBC, 0x00, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x81}} //  58
  // The output of channel should be high again
, {TIMER_TICK     ,  1,  0, (StepFunction *) _output4Set         , {}} //  59
, {END}
};
static Test_Case special_1_tc =
{
  "OUT8 - Special Function 1"
, 0x0004, 0x2060, 01
, 0 // power-on delay
, special_1_eepromSetup
, NULL
, (StateFunction *) _gatherState
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_special_1
};

TEST_CASE("OUT8 - Special Function 1","[SEPCIAL]")
{
  executeTest(& special_1_tc);
}
// <<< TC:special_1
#endif
